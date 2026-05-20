#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QUuid>
#include <QVector>
#include <QColor>
#include <QPointF>
#include <memory>

#include "PacketType.h"
#include "Protocol.h"

class DrawObject;

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer(const QString &host, quint16 port,
                         const QString &roomName = "default");
    void disconnectFromServer();
    bool isConnected() const;

public slots:
    void sendDraw (const std::shared_ptr<DrawObject> &obj);
    void sendErase(const QUuid &uuid);
    void sendFill (const QUuid &uuid, const QColor &color);
    void sendMove (const QUuid &uuid, const QPointF &delta);
    void sendClear();

signals:
    void connectedToServer();
    void disconnectedFromServer();
    void connectionError(const QString &error);

    void remoteDrawReceived  (std::shared_ptr<DrawObject> obj);
    void remoteEraseReceived (QUuid uuid);
    void remoteFillReceived  (QUuid uuid, QColor color);
    void remoteMoveReceived  (QUuid uuid, QPointF delta);
    void remoteClearReceived ();
    void snapshotReceived    (QVector<std::shared_ptr<DrawObject>> objects);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError err);

private:
    void sendPacket(PacketType type, const QByteArray &payload);
    void handlePacket(const Protocol::Packet &packet);

    QTcpSocket m_socket;
    QByteArray m_buffer;
    QString    m_roomName = "default";
};
