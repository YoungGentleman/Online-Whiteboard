#pragma once

#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QSpinBox>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>

#include "../canvas/CanvasView.h"
#include "../data/BoardModel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNewBoard();
    void onOpenFile();
    void onSaveFile();
    void onCreateRoom();
    void onJoinRoom();
    void onAbout();
    void onColorButtonClicked();
    void onPenWidthChanged(int value);

private:
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    static QWidget* makeSeparator();
    void updateWindowTitle(const QString &filename = {});
    void updateColorButton();

    BoardModel  *m_model  = nullptr;
    CanvasView  *m_canvas = nullptr;

    QLabel   *m_statusTool       = nullptr;
    QLabel   *m_statusZoom       = nullptr;
    QLabel   *m_statusConnection = nullptr;

    QAction  *m_actSelect    = nullptr;
    QAction  *m_actPencil    = nullptr;
    QAction  *m_actRect      = nullptr;
    QAction  *m_actEllipse   = nullptr;
    QAction  *m_actTriangle  = nullptr;
    QAction  *m_actEraser    = nullptr;
    QAction  *m_actFill      = nullptr;

    QPushButton *m_colorBtn  = nullptr;
    QSpinBox    *m_widthSpin = nullptr;
    QColor       m_currentColor = Qt::black;
};
