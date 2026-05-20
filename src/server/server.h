#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QList>
#include <QString>
#include <QByteArray>
#include <memory>

#include "../data/DrawObject.h"
#include "../network/Protocol.h"

struct ClientState {
    QString    roomName;
    QByteArray buffer;
};

struct Room {
    QList<QTcpSocket*>                   clients;
    QVector<std::shared_ptr<DrawObject>> objects;
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(quint16 port, QObject *parent = nullptr);
    ~Server() override;

    bool    isListening() const { return m_server.isListening(); }
    quint16 port()        const { return m_server.serverPort(); }

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void handlePacket(QTcpSocket *socket, const Protocol::Packet &packet);
    void sendSnapshot(QTcpSocket *socket, const QString &roomName);
    void broadcast(const QString &roomName,
                   const QByteArray &frame,
                   QTcpSocket *exclude = nullptr);

    QTcpServer                      m_server;
    QHash<QString, Room>            m_rooms;
    QHash<QTcpSocket*, ClientState> m_clients;
};
