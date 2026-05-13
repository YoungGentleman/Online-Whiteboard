#include "MainWindow.h"
#include "ConnectionDialog.h"

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

    // создаём модель и холст
    m_model  = new BoardModel(this);
    m_canvas = new CanvasView(m_model, this);
    setCentralWidget(m_canvas);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    updateWindowTitle();

    // подключаем сигналы холста к статусбару
    connect(m_canvas, &CanvasView::zoomChanged,
            this, [this](int pct) {
                m_statusZoom->setText(QString("%1%").arg(pct));
            });
    connect(m_canvas, &CanvasView::toolChanged,
            this, [this](const QString &name) {
                m_statusTool->setText(QString("Инструмент: %1").arg(name));
            });
}

// ---------------------------------------------------------------------------
// меню
// ---------------------------------------------------------------------------

void MainWindow::setupMenuBar()
{
    // --- Файл ---
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

    // --- Правка ---
    QMenu *editMenu = menuBar()->addMenu("&Правка");

    auto *actUndo  = new QAction("&Отменить",  this);
    auto *actRedo  = new QAction("&Повторить", this);
    auto *actClear = new QAction("Очистить всё", this);
    actUndo->setShortcut(QKeySequence::Undo);
    actRedo->setShortcut(QKeySequence::Redo);
    connect(actClear, &QAction::triggered, this, [this]() { m_model->clear(); });
    editMenu->addAction(actUndo);
    editMenu->addAction(actRedo);
    editMenu->addSeparator();
    editMenu->addAction(actClear);

    // --- Вид ---
    QMenu *viewMenu = menuBar()->addMenu("&Вид");

    auto *actZoomIn  = new QAction("Увеличить",        this);
    auto *actZoomOut = new QAction("Уменьшить",        this);
    auto *actFit     = new QAction("По размеру окна",  this);
    actZoomIn->setShortcut(QKeySequence::ZoomIn);
    actZoomOut->setShortcut(QKeySequence::ZoomOut);
    actFit->setShortcut(QKeySequence("Ctrl+0"));
    connect(actZoomIn,  &QAction::triggered, this, [this]() { m_canvas->zoomIn(); });
    connect(actZoomOut, &QAction::triggered, this, [this]() { m_canvas->zoomOut(); });
    connect(actFit,     &QAction::triggered, this, [this]() { m_canvas->fitToWindow(); });
    viewMenu->addAction(actZoomIn);
    viewMenu->addAction(actZoomOut);
    viewMenu->addAction(actFit);

    // --- Сеть ---
    QMenu *netMenu = menuBar()->addMenu("&Сеть");

    auto *actHost = new QAction("&Создать комнату (хост)…", this);
    auto *actJoin = new QAction("&Войти в комнату…",        this);
    connect(actHost, &QAction::triggered, this, &MainWindow::onCreateRoom);
    connect(actJoin, &QAction::triggered, this, &MainWindow::onJoinRoom);
    netMenu->addAction(actHost);
    netMenu->addAction(actJoin);

    // --- Помощь ---
    QMenu *helpMenu = menuBar()->addMenu("&Помощь");

    auto *actAbout = new QAction("&О программе", this);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(actAbout);
}

// ---------------------------------------------------------------------------
// тулбар
// ---------------------------------------------------------------------------

void MainWindow::setupToolBar()
{
    QToolBar *toolbar = addToolBar("Инструменты");
    toolbar->setObjectName("mainToolbar");
    toolbar->setMovable(false);

    // --- инструменты рисования ---
    m_actPencil   = toolbar->addAction("Карандаш");
    m_actRect     = toolbar->addAction("Прямоугольник");
    m_actEllipse  = toolbar->addAction("Эллипс");
    m_actTriangle = toolbar->addAction("Треугольник");
    m_actEraser   = toolbar->addAction("Ластик");

    for (QAction *a : {m_actPencil, m_actRect, m_actEllipse, m_actTriangle, m_actEraser})
        a->setCheckable(true);
    m_actPencil->setChecked(true);

    // переключение инструментов
    connect(m_actPencil,   &QAction::triggered, this, [this]() {
        m_canvas->setToolPencil();
        for (auto *a : {m_actPencil,m_actRect,m_actEllipse,m_actTriangle,m_actEraser}) a->setChecked(false);
        m_actPencil->setChecked(true);
    });
    connect(m_actRect,     &QAction::triggered, this, [this]() {
        m_canvas->setToolRect();
        for (auto *a : {m_actPencil,m_actRect,m_actEllipse,m_actTriangle,m_actEraser}) a->setChecked(false);
        m_actRect->setChecked(true);
    });
    connect(m_actEllipse,  &QAction::triggered, this, [this]() {
        m_canvas->setToolEllipse();
        for (auto *a : {m_actPencil,m_actRect,m_actEllipse,m_actTriangle,m_actEraser}) a->setChecked(false);
        m_actEllipse->setChecked(true);
    });
    connect(m_actTriangle, &QAction::triggered, this, [this]() {
        m_canvas->setToolTriangle();
        for (auto *a : {m_actPencil,m_actRect,m_actEllipse,m_actTriangle,m_actEraser}) a->setChecked(false);
        m_actTriangle->setChecked(true);
    });
    connect(m_actEraser,   &QAction::triggered, this, [this]() {
        m_canvas->setToolEraser();
        for (auto *a : {m_actPencil,m_actRect,m_actEllipse,m_actTriangle,m_actEraser}) a->setChecked(false);
        m_actEraser->setChecked(true);
    });

    toolbar->addSeparator();

    // --- цвет ---
    m_colorBtn = new QPushButton();
    m_colorBtn->setFixedSize(28, 28);
    m_colorBtn->setToolTip("Цвет линии");
    updateColorButton();
    connect(m_colorBtn, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);
    toolbar->addWidget(m_colorBtn);

    toolbar->addSeparator();

    // --- толщина линии ---
    auto *widthLabel = new QLabel(" Толщина: ");
    toolbar->addWidget(widthLabel);

    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(1, 20);
    m_widthSpin->setValue(2);
    m_widthSpin->setToolTip("Толщина линии");
    connect(m_widthSpin, &QSpinBox::valueChanged, this, &MainWindow::onPenWidthChanged);
    toolbar->addWidget(m_widthSpin);
}

// ---------------------------------------------------------------------------
// статусбар
// ---------------------------------------------------------------------------

void MainWindow::setupStatusBar()
{
    m_statusTool = new QLabel("Инструмент: Карандаш");
    m_statusTool->setMinimumWidth(180);

    m_statusZoom = new QLabel("100%");
    m_statusZoom->setMinimumWidth(60);

    m_statusConnection = new QLabel("Оффлайн");
    m_statusConnection->setMinimumWidth(140);

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
    // красим кнопку в текущий цвет
    QString style = QString("background-color: %1; border: 1px solid #888;")
                        .arg(m_currentColor.name());
    m_colorBtn->setStyleSheet(style);
}

// ---------------------------------------------------------------------------
// слоты
// ---------------------------------------------------------------------------

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void MainWindow::onNewBoard()
{
    m_model->clear();
    updateWindowTitle();
}

void MainWindow::onOpenFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Открыть доску", {},
        "Файлы доски (*.wbd);;Все файлы (*)"
    );
    if (path.isEmpty()) return;
    // TODO коммит 3: загрузка через Serializer
    updateWindowTitle(QFileInfo(path).fileName());
}

void MainWindow::onSaveFile()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "Сохранить доску", {},
        "Файлы доски (*.wbd);;Все файлы (*)"
    );
    if (path.isEmpty()) return;
    // TODO коммит 3: сохранение через Serializer
}

void MainWindow::onCreateRoom()
{
    ConnectionDialog dlg(ConnectionDialog::Mode::Host, this);
    dlg.exec();
    // TODO коммит 3: передать настройки в NetworkManager
}

void MainWindow::onJoinRoom()
{
    ConnectionDialog dlg(ConnectionDialog::Mode::Client, this);
    dlg.exec();
    // TODO коммит 3: передать настройки в NetworkManager
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "О программе",
        QString("<b>Whiteboard</b> v0.1<br>"
                "Совместная доска для рисования.<br>"
                "Qt %1 / C++17").arg(QT_VERSION_STR)
    );
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
