#include "MainWindow.h"
#include "ConnectionDialog.h"
#include "../server/server.h"
#include "../data/Serializer.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QFrame>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(900, 600);
    resize(1280, 800);

    m_model  = new BoardModel(this);
    m_canvas = new CanvasView(m_model, this);
    setCentralWidget(m_canvas);

    m_network = new NetworkManager(this);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    updateWindowTitle();

    connect(m_canvas, &CanvasView::zoomChanged,
            this, [this](int pct) {
                m_statusZoom->setText(QString("%1%").arg(pct));
            });
    connect(m_canvas, &CanvasView::toolChanged,
            this, [this](const QString &name) {
                m_statusTool->setText(QString("Инструмент: %1").arg(name));
            });

    connect(m_model, &BoardModel::localObjectCreated,
            this, &MainWindow::onLocalObjectCreated);
    connect(m_model, &BoardModel::localObjectErased,
            this, &MainWindow::onLocalObjectErased);
    connect(m_model, &BoardModel::localObjectFilled,
            this, &MainWindow::onLocalObjectFilled);
    connect(m_model, &BoardModel::localObjectMoved,
            this, &MainWindow::onLocalObjectMoved);

    connect(m_network, &NetworkManager::connectedToServer,
            this, &MainWindow::onNetConnected);
    connect(m_network, &NetworkManager::disconnectedFromServer,
            this, &MainWindow::onNetDisconnected);
    connect(m_network, &NetworkManager::connectionError,
            this, &MainWindow::onNetError);
    connect(m_network, &NetworkManager::snapshotReceived,
            this, &MainWindow::onSnapshotReceived);
    connect(m_network, &NetworkManager::remoteDrawReceived,
            this, &MainWindow::onRemoteDraw);
    connect(m_network, &NetworkManager::remoteEraseReceived,
            this, &MainWindow::onRemoteErase);
    connect(m_network, &NetworkManager::remoteFillReceived,
            this, &MainWindow::onRemoteFill);
    connect(m_network, &NetworkManager::remoteMoveReceived,
            this, &MainWindow::onRemoteMove);
    connect(m_network, &NetworkManager::remoteClearReceived,
            this, &MainWindow::onRemoteClear);
}

MainWindow::~MainWindow()
{
    teardownNetwork();
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&Файл");
    auto *actNew  = new QAction("&Новая доска", this);
    auto *actOpen = new QAction("&Открыть…",    this);
    auto *actSave = new QAction("&Сохранить",   this);
    auto *actQuit = new QAction("&Выход",        this);
    actNew->setShortcut(QKeySequence::New);
    actOpen->setShortcut(QKeySequence::Open);
    actSave->setShortcut(QKeySequence::Save);
    actQuit->setShortcut(QKeySequence::Quit);
    connect(actNew,  &QAction::triggered, this, &MainWindow::onNewBoard);
    connect(actOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(actSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(actQuit, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(actNew);
    fileMenu->addAction(actOpen);
    fileMenu->addAction(actSave);
    fileMenu->addSeparator();
    fileMenu->addAction(actQuit);

    QMenu *editMenu = menuBar()->addMenu("&Правка");
    auto *actClear = new QAction("Очистить всё", this);
    connect(actClear, &QAction::triggered, this, &MainWindow::onClearBoard);
    editMenu->addAction(actClear);

    QMenu *viewMenu = menuBar()->addMenu("&Вид");
    auto *actZoomIn  = new QAction("Увеличить",       this);
    auto *actZoomOut = new QAction("Уменьшить",       this);
    auto *actFit     = new QAction("По размеру окна", this);
    actZoomIn->setShortcut(QKeySequence::ZoomIn);
    actZoomOut->setShortcut(QKeySequence::ZoomOut);
    actFit->setShortcut(QKeySequence("Ctrl+0"));
    connect(actZoomIn,  &QAction::triggered, this, [this]() { m_canvas->zoomIn(); });
    connect(actZoomOut, &QAction::triggered, this, [this]() { m_canvas->zoomOut(); });
    connect(actFit,     &QAction::triggered, this, [this]() { m_canvas->fitToWindow(); });
    viewMenu->addAction(actZoomIn);
    viewMenu->addAction(actZoomOut);
    viewMenu->addAction(actFit);

    QMenu *netMenu = menuBar()->addMenu("&Сеть");
    auto *actHost = new QAction("&Создать комнату (хост)…", this);
    auto *actJoin = new QAction("&Войти в комнату…",        this);
    m_actLeaveRoom = new QAction("&Покинуть комнату",       this);
    m_actLeaveRoom->setEnabled(false);
    connect(actHost,        &QAction::triggered, this, &MainWindow::onCreateRoom);
    connect(actJoin,        &QAction::triggered, this, &MainWindow::onJoinRoom);
    connect(m_actLeaveRoom, &QAction::triggered, this, &MainWindow::onLeaveRoom);
    netMenu->addAction(actHost);
    netMenu->addAction(actJoin);
    netMenu->addSeparator();
    netMenu->addAction(m_actLeaveRoom);

    QMenu *helpMenu = menuBar()->addMenu("&Помощь");
    auto *actAbout = new QAction("&О программе", this);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(actAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolbar = addToolBar("Инструменты");
    toolbar->setObjectName("mainToolbar");
    toolbar->setMovable(false);
    toolbar->setContextMenuPolicy(Qt::PreventContextMenu);

    m_actSelect   = toolbar->addAction("Выделение");
    m_actPencil   = toolbar->addAction("Карандаш");
    m_actRect     = toolbar->addAction("Прямоугольник");
    m_actEllipse  = toolbar->addAction("Эллипс");
    m_actTriangle = toolbar->addAction("Треугольник");
    m_actEraser   = toolbar->addAction("Ластик");
    m_actFill     = toolbar->addAction("Заливка");

    const auto allTools = [this]() {
        return QList<QAction*>{m_actSelect, m_actPencil, m_actRect,
                               m_actEllipse, m_actTriangle, m_actEraser, m_actFill};
    };

    for (QAction *a : allTools()) a->setCheckable(true);
    m_actPencil->setChecked(true);

    connect(m_actSelect, &QAction::triggered, this, [=]() {
        for (auto *a : allTools()) a->setChecked(false);
        m_actSelect->setChecked(true); m_canvas->setToolSelect();
    });
    connect(m_actPencil, &QAction::triggered, this, [=]() {
        for (auto *a : allTools()) a->setChecked(false);
        m_actPencil->setChecked(true); m_canvas->setToolPencil();
    });
    connect(m_actRect, &QAction::triggered, this, [=]() {
        for (auto *a : allTools()) a->setChecked(false);
        m_actRect->setChecked(true); m_canvas->setToolRect();
    });
    connect(m_actEllipse, &QAction::triggered, this, [=]() {
        for (auto *a : allTools()) a->setChecked(false);
        m_actEllipse->setChecked(true); m_canvas->setToolEllipse();
    });
    connect(m_actTriangle, &QAction::triggered, this, [=]() {
        for (auto *a : allTools()) a->setChecked(false);
        m_actTriangle->setChecked(true); m_canvas->setToolTriangle();
    });
    connect(m_actEraser, &QAction::triggered, this, [=]() {
        for (auto *a : allTools()) a->setChecked(false);
        m_actEraser->setChecked(true); m_canvas->setToolEraser();
    });
    connect(m_actFill, &QAction::triggered, this, [=]() {
        for (auto *a : allTools()) a->setChecked(false);
        m_actFill->setChecked(true); m_canvas->setToolFill();
    });

    toolbar->addSeparator();

    m_colorBtn = new QPushButton();
    m_colorBtn->setFixedSize(28, 28);
    m_colorBtn->setToolTip("Цвет линии");
    updateColorButton();
    connect(m_colorBtn, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);
    toolbar->addWidget(m_colorBtn);

    toolbar->addSeparator();
    toolbar->addWidget(new QLabel(" Толщина: "));

    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(1, 20);
    m_widthSpin->setValue(2);
    m_widthSpin->setToolTip("Толщина линии");
    connect(m_widthSpin, &QSpinBox::valueChanged, this, &MainWindow::onPenWidthChanged);
    toolbar->addWidget(m_widthSpin);
}

void MainWindow::setupStatusBar()
{
    m_statusTool = new QLabel("Инструмент: Карандаш");
    m_statusTool->setMinimumWidth(180);
    m_statusZoom = new QLabel("100%");
    m_statusZoom->setMinimumWidth(60);
    m_statusConnection = new QLabel("Оффлайн");
    m_statusConnection->setMinimumWidth(220);
    statusBar()->addWidget(m_statusTool);
    statusBar()->addWidget(makeSeparator());
    statusBar()->addWidget(m_statusZoom);
    statusBar()->addPermanentWidget(m_statusConnection);
}

QWidget* MainWindow::makeSeparator()
{
    auto *sep = new QFrame();
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Sunken);
    return sep;
}

void MainWindow::updateWindowTitle(const QString &filename)
{
    setWindowTitle(filename.isEmpty()
        ? "Без названия — Whiteboard"
        : QString("%1 — Whiteboard").arg(filename));
}

void MainWindow::updateColorButton()
{
    m_colorBtn->setStyleSheet(
        QString("background-color: %1; border: 1px solid #888;")
            .arg(m_currentColor.name()));
}

void MainWindow::updateConnectionStatus(const QString &text, bool ok)
{
    m_statusConnection->setText(text);
    m_statusConnection->setStyleSheet(ok ? "color: #16803c;" : "color: #b00020;");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    teardownNetwork();
    event->accept();
}

void MainWindow::onNewBoard()
{
    onClearBoard();
    updateWindowTitle();
}

void MainWindow::onOpenFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Открыть доску", {}, "Файлы доски (*.wbd);;Все файлы (*.*)");
    if (path.isEmpty()) return;

    auto objects = Serializer::loadFromFile(path);
    if (objects.isEmpty()) {
        QMessageBox::warning(this, "Открытие файла",
            "Не удалось прочитать файл или он пустой.");
        return;
    }

    m_applyingRemote = true;
    m_model->clear();
    for (auto &obj : objects)
        m_model->addObject(obj, true);
    m_applyingRemote = false;

    updateWindowTitle(QFileInfo(path).fileName());
}

void MainWindow::onSaveFile()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить доску", {}, "Файлы доски (*.wbd);;Все файлы (*.*)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".wbd", Qt::CaseInsensitive))
        path += ".wbd";

    m_canvas->flushSelectMode();
    if (!Serializer::saveToFile(path, m_model->objects())) {
        QMessageBox::warning(this, "Сохранение", "Не удалось сохранить файл.");
        return;
    }
    updateWindowTitle(QFileInfo(path).fileName());
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "О программе",
        QString("<b>Whiteboard</b> v0.2<br>"
                "Совместная доска для рисования.<br>"
                "Qt %1 / C++17").arg(QT_VERSION_STR));
}

void MainWindow::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(m_currentColor, this, "Выберите цвет");
    if (!color.isValid()) return;
    m_currentColor = color;
    m_canvas->setColor(color);
    updateColorButton();
}

void MainWindow::onPenWidthChanged(int value)
{
    m_canvas->setPenWidth(value);
}

void MainWindow::onClearBoard()
{
    if (m_network && m_network->isConnected() && !m_applyingRemote)
        m_network->sendClear();
    m_applyingRemote = true;
    m_model->clear();
    m_applyingRemote = false;
}

void MainWindow::onCreateRoom()
{
    ConnectionDialog dlg(ConnectionDialog::Mode::Host, this);
    if (dlg.exec() != QDialog::Accepted) return;
    startHosting(quint16(dlg.port()), dlg.roomName());
}

void MainWindow::onJoinRoom()
{
    ConnectionDialog dlg(ConnectionDialog::Mode::Client, this);
    if (dlg.exec() != QDialog::Accepted) return;
    joinAsClient(dlg.hostAddress(), quint16(dlg.port()), dlg.roomName());
}

void MainWindow::onLeaveRoom()
{
    teardownNetwork();
    updateConnectionStatus("Оффлайн", false);
    m_actLeaveRoom->setEnabled(false);
}

void MainWindow::startHosting(quint16 port, const QString &roomName)
{
    teardownNetwork();

    m_localServer = new Server(port, this);
    if (!m_localServer->isListening()) {
        QMessageBox::critical(this, "Ошибка",
            QString("Не удалось открыть порт %1.\n"
                    "Возможно, порт уже занят другим приложением.").arg(port));
        m_localServer->deleteLater();
        m_localServer = nullptr;
        return;
    }

    updateConnectionStatus(QString("Хост: запуск (порт %1, '%2')…")
                               .arg(port).arg(roomName), true);
    m_network->connectToServer("127.0.0.1", port, roomName);
}

void MainWindow::joinAsClient(const QString &host, quint16 port, const QString &roomName)
{
    teardownNetwork();
    m_applyingRemote = true;
    m_model->clear();
    m_applyingRemote = false;
    updateConnectionStatus(QString("Подключение к %1:%2…").arg(host).arg(port), true);
    m_network->connectToServer(host, port, roomName);
}

void MainWindow::teardownNetwork()
{
    if (m_network && m_network->isConnected())
        m_network->disconnectFromServer();
    if (m_localServer) {
        m_localServer->deleteLater();
        m_localServer = nullptr;
    }
}

void MainWindow::onNetConnected()
{
    const QString role = m_localServer ? "Хост" : "Клиент";
    updateConnectionStatus(QString("%1: подключён").arg(role), true);
    m_actLeaveRoom->setEnabled(true);

    if (m_localServer) {
        for (const auto &obj : m_model->objects())
            m_network->sendDraw(obj);
    }
}

void MainWindow::onNetDisconnected()
{
    updateConnectionStatus("Оффлайн", false);
    m_actLeaveRoom->setEnabled(false);
}

void MainWindow::onNetError(const QString &msg)
{
    updateConnectionStatus(QString("Ошибка: %1").arg(msg), false);
}

void MainWindow::onSnapshotReceived(QVector<std::shared_ptr<DrawObject>> objects)
{
    m_applyingRemote = true;
    m_model->clear();
    for (const auto &obj : objects)
        m_model->addObject(obj, true);
    m_applyingRemote = false;
}

void MainWindow::onRemoteDraw(std::shared_ptr<DrawObject> obj)
{
    m_applyingRemote = true;
    if (m_model->findByUuid(obj->uuid))
        m_model->removeObject(obj->uuid, true);
    m_model->addObject(obj, true);
    m_applyingRemote = false;
}

void MainWindow::onRemoteErase(QUuid uuid)
{
    m_applyingRemote = true;
    m_model->removeObject(uuid, true);
    m_applyingRemote = false;
}

void MainWindow::onRemoteFill(QUuid uuid, QColor color)
{
    m_applyingRemote = true;
    m_model->fillObject(uuid, color, true);
    m_applyingRemote = false;
}

void MainWindow::onRemoteMove(QUuid uuid, QPointF delta)
{
    m_applyingRemote = true;
    m_model->updateObjectPosition(uuid, delta, true);
    m_applyingRemote = false;
}

void MainWindow::onRemoteClear()
{
    m_applyingRemote = true;
    m_model->clear();
    m_applyingRemote = false;
}

void MainWindow::onLocalObjectCreated(std::shared_ptr<DrawObject> obj)
{
    if (m_applyingRemote) return;
    if (m_network->isConnected())
        m_network->sendDraw(obj);
}

void MainWindow::onLocalObjectErased(QUuid uuid)
{
    if (m_applyingRemote) return;
    if (m_network->isConnected())
        m_network->sendErase(uuid);
}

void MainWindow::onLocalObjectFilled(QUuid uuid, QColor color)
{
    if (m_applyingRemote) return;
    if (m_network->isConnected())
        m_network->sendFill(uuid, color);
}

void MainWindow::onLocalObjectMoved(QUuid uuid, QPointF delta)
{
    if (m_applyingRemote) return;
    if (m_network->isConnected())
        m_network->sendMove(uuid, delta);
}
