#include "MainWindow.h"
#include "ConnectionDialog.h"
#include "../server/Server.h"

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

    connect(m_canvas, &CanvasView::objectCreated,
            this, &MainWindow::onLocalObjectCreated);
    connect(m_canvas, &CanvasView::objectErasedLocal,
            this, &MainWindow::onLocalObjectErased);

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
    auto *actOpen = new QAction("&Открыть…",     this);
    auto *actSave = new QAction("&Сохранить",    this);
    auto *actQuit = new QAction("&Выход",         this);

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

    m_actPencil   = toolbar->addAction("Карандаш");
    m_actRect     = toolbar->addAction("Прямоугольник");
    m_actEllipse  = toolbar->addAction("Эллипс");
    m_actTriangle = toolbar->addAction("Треугольник");
    m_actEraser   = toolbar->addAction("Ластик");

    const QList<QAction*> toolActions = {
        m_actPencil, m_actRect, m_actEllipse, m_actTriangle, m_actEraser
    };
    for (QAction *a : toolActions)
        a->setCheckable(true);
    m_actPencil->setChecked(true);

    auto uncheckAll = [toolActions]() {
        for (auto *a : toolActions) a->setChecked(false);
    };

    connect(m_actPencil,   &QAction::triggered, this, [this, uncheckAll]() {
        m_canvas->setToolPencil();
        uncheckAll(); m_actPencil->setChecked(true);
    });
    connect(m_actRect, &QAction::triggered, this, [this, uncheckAll]() {
        m_canvas->setToolRect();
        uncheckAll(); m_actRect->setChecked(true);
    });
    connect(m_actEllipse, &QAction::triggered, this, [this, uncheckAll]() {
        m_canvas->setToolEllipse();
        uncheckAll(); m_actEllipse->setChecked(true);
    });
    connect(m_actTriangle, &QAction::triggered, this, [this, uncheckAll]() {
        m_canvas->setToolTriangle();
        uncheckAll(); m_actTriangle->setChecked(true);
    });
    connect(m_actEraser, &QAction::triggered, this, [this, uncheckAll]() {
        m_canvas->setToolEraser();
        uncheckAll(); m_actEraser->setChecked(true);
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
    QString title = filename.isEmpty()
        ? "Без названия — Whiteboard"
        : QString("%1 — Whiteboard").arg(filename);
    setWindowTitle(title);
}

void MainWindow::updateColorButton()
{
    QString style = QString("background-color: %1; border: 1px solid #888;")
                        .arg(m_currentColor.name());
    m_colorBtn->setStyleSheet(style);
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
        this, "Открыть доску", {},
        "Файлы доски (*.wbd);;Все файлы (*)");
    if (path.isEmpty()) return;
    updateWindowTitle(QFileInfo(path).fileName());
}

void MainWindow::onSaveFile()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "Сохранить доску", {},
        "Файлы доски (*.wbd);;Все файлы (*)");
    if (path.isEmpty()) return;
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "О программе",
        QString("<b>Whiteboard</b> v0.1<br>"
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
    m_model->clear();
    if (m_network && m_network->isConnected())
        m_network->sendClear();
}

// ---------------------------------------------------------------------------
// слоты «Сеть»
// ---------------------------------------------------------------------------

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
                    "Возможно, порт уже занят другим приложением").arg(port));
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
    m_model->clear();

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
        m_model->addObject(obj);
    m_applyingRemote = false;
}

void MainWindow::onRemoteDraw(std::shared_ptr<DrawObject> obj)
{
    m_applyingRemote = true;
    m_model->addObject(obj);
    m_applyingRemote = false;
}

void MainWindow::onRemoteErase(QUuid uuid)
{
    m_applyingRemote = true;
    m_model->removeObject(uuid);
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