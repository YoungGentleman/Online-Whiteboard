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
#include <memory>

#include "../canvas/CanvasView.h"
#include "../data/BoardModel.h"
#include "../network/NetworkManager.h"

class Server;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNewBoard();
    void onOpenFile();
    void onSaveFile();

    void onCreateRoom();
    void onJoinRoom();
    void onLeaveRoom();

    void onAbout();
    void onColorButtonClicked();
    void onPenWidthChanged(int value);
    void onClearBoard();

    void onNetConnected();
    void onNetDisconnected();
    void onNetError(const QString &msg);
    void onSnapshotReceived(QVector<std::shared_ptr<DrawObject>> objects);
    void onRemoteDraw (std::shared_ptr<DrawObject> obj);
    void onRemoteErase(QUuid uuid);
    void onRemoteFill (QUuid uuid, QColor color);
    void onRemoteMove (QUuid uuid, QPointF delta);
    void onRemoteClear();
    void onUserJoined(int totalCount);
    void onUserLeft(int totalCount);

    void onLocalObjectCreated(std::shared_ptr<DrawObject> obj);
    void onLocalObjectErased (QUuid uuid);
    void onLocalObjectFilled (QUuid uuid, QColor color);
    void onLocalObjectMoved  (QUuid uuid, QPointF delta);

private:
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    static QWidget* makeSeparator();
    void updateWindowTitle(const QString &filename = {});
    void updateColorButton();
    void updateConnectionStatus(const QString &text, bool ok);
    void refreshConnectionStatus();
    QString detectLocalIp() const;

    void startHosting(quint16 port, const QString &roomName);
    void joinAsClient(const QString &host, quint16 port, const QString &roomName);
    void teardownNetwork();

    BoardModel  *m_model  = nullptr;
    CanvasView  *m_canvas = nullptr;

    NetworkManager *m_network     = nullptr;
    Server         *m_localServer = nullptr;
    bool            m_applyingRemote = false;

    QString  m_currentHost;
    quint16  m_currentPort = 0;
    QString  m_currentRoom;
    int      m_userCount   = 0;
    bool     m_isHostRole  = false;
    bool     m_intentionalDisconnect = false;
    bool     m_connectionErrorShown = false;

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

    QAction  *m_actLeaveRoom = nullptr;

    QPushButton *m_colorBtn  = nullptr;
    QSpinBox    *m_widthSpin = nullptr;
    QColor       m_currentColor = Qt::black;
};
