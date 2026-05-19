#include "MainWindow.h"
#include "ConnectionDialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QFrame>
#include <QColorDialog>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>

#include "../data/Serializer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(900, 600);
    resize(1280, 800);

    m_model  = new BoardModel(this);
    m_canvas = new CanvasView(m_model, this);
    setCentralWidget(m_canvas);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    updateWindowTitle();

    connect(m_canvas, &CanvasView::zoomChanged,
            this, [this](int pct) { m_statusZoom->setText(QString("%1%").arg(pct)); });
    connect(m_canvas, &CanvasView::toolChanged,
            this, [this](const QString &name) {
                m_statusTool->setText(QString("Инструмент: %1").arg(name));
            });
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
    auto *actUndo  = new QAction("&Отменить",    this);
    auto *actRedo  = new QAction("&Повторить",   this);
    auto *actClear = new QAction("Очистить всё", this);
    actUndo->setShortcut(QKeySequence::Undo);
    actRedo->setShortcut(QKeySequence::Redo);
    connect(actClear, &QAction::triggered, this, [this]() { m_model->clear(); });
    editMenu->addAction(actUndo);
    editMenu->addAction(actRedo);
    editMenu->addSeparator();
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
    connect(actHost, &QAction::triggered, this, &MainWindow::onCreateRoom);
    connect(actJoin, &QAction::triggered, this, &MainWindow::onJoinRoom);
    netMenu->addAction(actHost);
    netMenu->addAction(actJoin);

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

void MainWindow::closeEvent(QCloseEvent *event) { event->accept(); }

void MainWindow::onNewBoard() { m_model->clear(); updateWindowTitle(); }

void MainWindow::onOpenFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Открыть доску", {}, "Файлы доски (*.wbd);;Все файлы (*.*)");
    if (path.isEmpty()) return;
    auto objects = Serializer::loadFromFile(path);
    m_model->clear();
    for (auto &obj : objects)
        m_model->addObject(obj, true);
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

    Serializer::saveToFile(path, m_model->objects());
    updateWindowTitle(QFileInfo(path).fileName());
}

void MainWindow::onCreateRoom()
{
    ConnectionDialog dlg(ConnectionDialog::Mode::Host, this);
    dlg.exec();
}

void MainWindow::onJoinRoom()
{
    ConnectionDialog dlg(ConnectionDialog::Mode::Client, this);
    dlg.exec();
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

void MainWindow::onPenWidthChanged(int value) { m_canvas->setPenWidth(value); }
