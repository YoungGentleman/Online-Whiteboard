#include "server.h"

#include <QDataStream>
#include <QDebug>
#include <QHostAddress>
#include <QMutexLocker>

#include "../data/Serializer.h"

// ---------------------------------------------------------------------------
// RoomWorker
// ---------------------------------------------------------------------------

RoomWorker::RoomWorker(const QString &roomName, QObject *parent)
    : QObject(parent), m_roomName(roomName)
{
}

void RoomWorker::addClient(QTcpSocket *socket)
{
    QMutexLocker lock(&m_mutex);
    if (!m_clients.contains(socket))
        m_clients.append(socket);
}

void RoomWorker::removeClient(QTcpSocket *socket)
{
    QMutexLocker lock(&m_mutex);
    m_clients.removeAll(socket);
    qInfo() << "[Room" << m_roomName << "] клиент вышел, осталось:" << m_clients.size();

    if (!m_clients.isEmpty()) {
        QByteArray payload;
        QDataStream ds(&payload, QIODevice::WriteOnly);
        ds.setVersion(QDataStream::Qt_6_0);
        ds << qint32(m_clients.size());
        broadcast(Protocol::makePacket(PacketType::UserLeft,
                                       VectorClock{}, payload));
    }

    if (m_clients.isEmpty())
        emit roomEmpty(m_roomName);
}

bool RoomWorker::hasClients() const
{
    QMutexLocker lock(&m_mutex);
    return !m_clients.isEmpty();
}

void RoomWorker::handlePacket(QTcpSocket *socket, const Protocol::Packet &packet)
{
    QMutexLocker lock(&m_mutex);

    switch (packet.type) {

    case PacketType::Join: {
        sendSnapshot(socket);

        QByteArray payload;
        QDataStream ds(&payload, QIODevice::WriteOnly);
        ds.setVersion(QDataStream::Qt_6_0);
        ds << qint32(m_clients.size());
        broadcast(Protocol::makePacket(PacketType::UserJoined,
                                       VectorClock{}, payload));
        qInfo() << "[Room" << m_roomName << "] новый клиент, всего:" << m_clients.size();
        break;
    }

    case PacketType::Draw: {
        auto obj = Serializer::deserializeObject(packet.payload);
        if (obj) {
            bool replaced = false;
            for (int i = 0; i < m_objects.size(); ++i) {
                if (m_objects[i]->uuid == obj->uuid) {
                    m_objects[i] = obj;
                    replaced = true;
                    break;
                }
            }
            if (!replaced) m_objects.append(obj);
        }
        broadcast(Protocol::makePacket(PacketType::Draw,
                                       packet.clock, packet.payload), socket);
        break;
    }

    case PacketType::Erase: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; in >> uuid;
        for (int i = 0; i < m_objects.size(); ++i)
            if (m_objects[i]->uuid == uuid) { m_objects.removeAt(i); break; }
        broadcast(Protocol::makePacket(PacketType::Erase,
                                       packet.clock, packet.payload), socket);
        break;
    }

    case PacketType::Fill: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; QColor color;
        in >> uuid >> color;
        for (auto &obj : m_objects)
            if (obj->uuid == uuid) { obj->filled = true; obj->fillColor = color; break; }
        broadcast(Protocol::makePacket(PacketType::Fill,
                                       packet.clock, packet.payload), socket);
        break;
    }

    case PacketType::Move: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; QPointF delta;
        in >> uuid >> delta;
        for (auto &obj : m_objects) {
            if (obj->uuid != uuid) continue;
            switch (obj->type) {
            case ObjectType::Stroke:
                static_cast<StrokeObject*>(obj.get())->path.translate(delta); break;
            case ObjectType::Rect:
                static_cast<RectObject*>(obj.get())->rect.translate(delta); break;
            case ObjectType::Ellipse:
                static_cast<EllipseObject*>(obj.get())->rect.translate(delta); break;
            case ObjectType::Triangle: {
                auto *t = static_cast<TriangleObject*>(obj.get());
                t->p1 += delta; t->p2 += delta; t->p3 += delta; break;
            }
            }
            break;
        }
        broadcast(Protocol::makePacket(PacketType::Move,
                                       packet.clock, packet.payload), socket);
        break;
    }

    case PacketType::Clear:
        m_objects.clear();
        broadcast(Protocol::makePacket(PacketType::Clear,
                                       packet.clock, QByteArray()), socket);
        break;

    case PacketType::Ping:
        if (socket->state() == QAbstractSocket::ConnectedState)
            socket->write(Protocol::makePacket(PacketType::Pong,
                                               VectorClock{}, QByteArray()));
        break;

    default:
        break;
    }
}

void RoomWorker::sendSnapshot(QTcpSocket *socket)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << qint32(m_objects.size());
    for (const auto &obj : m_objects)
        out << Serializer::serializeObject(*obj);
    socket->write(Protocol::makePacket(PacketType::Snapshot,
                                       VectorClock{}, payload));
    qInfo() << "[Room" << m_roomName << "] снэпшот отправлен, объектов:" << m_objects.size();
}

void RoomWorker::broadcast(const QByteArray &frame, QTcpSocket *exclude)
{
    for (QTcpSocket *client : m_clients) {
        if (client == exclude) continue;
        if (client->state() != QAbstractSocket::ConnectedState) continue;
        client->write(frame);
    }
}

// ---------------------------------------------------------------------------
// Server
// ---------------------------------------------------------------------------

Server::Server(quint16 port, QObject *parent) : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection,
            this, &Server::onNewConnection);

    if (!m_server.listen(QHostAddress::Any, port)) {
        qCritical() << "[Server] не удалось стартовать на порту" << port
                    << ":" << m_server.errorString();
        return;
    }
    qInfo() << "[Server] слушает порт" << port;
}

Server::~Server()
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
        it.key()->abort();
    m_server.close();

    for (auto *thread : m_threads) {
        thread->quit();
        thread->wait();
    }
}

void Server::onNewConnection()
{
    while (m_server.hasPendingConnections()) {
        QTcpSocket *socket = m_server.nextPendingConnection();
        m_clients.insert(socket, ClientState{});

        connect(socket, &QTcpSocket::readyRead,
                this, &Server::onReadyRead);
        connect(socket, &QTcpSocket::disconnected,
                this, &Server::onDisconnected);

        qInfo() << "[Server] клиент подключился:"
                << socket->peerAddress().toString() << ":" << socket->peerPort();
    }
}

void Server::onReadyRead()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    auto it = m_clients.find(socket);
    if (it == m_clients.end()) return;

    it.value().buffer.append(socket->readAll());

    Protocol::Packet packet;
    while (Protocol::tryExtractPacket(it.value().buffer, packet)) {

        if (packet.type == PacketType::Join) {
            QDataStream in(packet.payload);
            in.setVersion(QDataStream::Qt_6_0);
            QString roomName;
            in >> roomName;
            if (roomName.isEmpty()) roomName = QStringLiteral("default");

            it.value().roomName = roomName;
            RoomWorker *worker = getOrCreateRoom(roomName);
            worker->addClient(socket);

            // передаём Join-пакет воркеру чтобы он отправил снэпшот
            QMetaObject::invokeMethod(worker, "handlePacket",
                                      Qt::QueuedConnection,
                                      Q_ARG(QTcpSocket*, socket),
                                      Q_ARG(Protocol::Packet, packet));
        } else {
            const QString &roomName = it.value().roomName;
            if (!roomName.isEmpty() && m_workers.contains(roomName)) {
                RoomWorker *worker = m_workers[roomName];
                QMetaObject::invokeMethod(worker, "handlePacket",
                                          Qt::QueuedConnection,
                                          Q_ARG(QTcpSocket*, socket),
                                          Q_ARG(Protocol::Packet, packet));
            }
        }
    }
}

void Server::onDisconnected()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    auto it = m_clients.find(socket);
    if (it != m_clients.end()) {
        const QString roomName = it.value().roomName;
        if (!roomName.isEmpty() && m_workers.contains(roomName)) {
            RoomWorker *worker = m_workers[roomName];
            QMetaObject::invokeMethod(worker, "removeClient",
                                      Qt::QueuedConnection,
                                      Q_ARG(QTcpSocket*, socket));
        }
        m_clients.erase(it);
    }

    socket->deleteLater();
}

void Server::onRoomEmpty(const QString &roomName)
{
    qInfo() << "[Server] комната" << roomName << "пуста — удаляем поток";

    if (m_workers.contains(roomName)) {
        m_workers[roomName]->deleteLater();
        m_workers.remove(roomName);
    }
    if (m_threads.contains(roomName)) {
        m_threads[roomName]->quit();
        m_threads[roomName]->wait();
        m_threads[roomName]->deleteLater();
        m_threads.remove(roomName);
    }
}

RoomWorker* Server::getOrCreateRoom(const QString &roomName)
{
    if (m_workers.contains(roomName))
        return m_workers[roomName];

    auto *thread = new QThread(this);
    auto *worker = new RoomWorker(roomName);
    worker->moveToThread(thread);

    connect(worker, &RoomWorker::roomEmpty,
            this, &Server::onRoomEmpty);

    thread->start();
    m_threads[roomName] = thread;
    m_workers[roomName] = worker;

    qInfo() << "[Server] создана комната" << roomName << "в новом потоке";
    return worker;
}
