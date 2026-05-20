#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QString>
#include <QByteArray>
#include <memory>

#include "../data/DrawObject.h"
#include "../data/PacketType.h"
#include "network/Protocol.h"

struct ClientState {
    QString    roomName;
    QByteArray buffer;
};

// RoomWorker живёт в отдельном потоке и обрабатывает все пакеты одной комнаты.
// Главный поток (Server) только маршрутизирует пакеты через сигналы.
class RoomWorker : public QObject
{
    Q_OBJECT
public:
    explicit RoomWorker(const QString &roomName, QObject *parent = nullptr);

    void addClient(QTcpSocket *socket);
    void removeClient(QTcpSocket *socket);
    bool hasClients() const;

signals:
    void roomEmpty(const QString &roomName);

public slots:
    void handlePacket(QTcpSocket *socket, const Protocol::Packet &packet);

private:
    void sendSnapshot(QTcpSocket *socket);
    void broadcast(const QByteArray &frame, QTcpSocket *exclude = nullptr);

    QString                              m_roomName;
    QList<QTcpSocket*>                   m_clients;
    QVector<std::shared_ptr<DrawObject>> m_objects;
    mutable QMutex                       m_mutex;
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(quint16 port, QObject *parent = nullptr);
    ~Server() override;

    bool    isListening() const { return m_server.isListening(); }
    quint16 port()        const { return m_server.serverPort(); }

signals:
    void dispatchPacket(QTcpSocket *socket, const Protocol::Packet &packet);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onRoomEmpty(const QString &roomName);

private:
    RoomWorker* getOrCreateRoom(const QString &roomName);

    QTcpServer                      m_server;
    QHash<QTcpSocket*, ClientState> m_clients;

    QHash<QString, RoomWorker*> m_workers;
    QHash<QString, QThread*>    m_threads;
};
