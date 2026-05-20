#include "server.h"

#include "../data/Serializer.h"
#include "../network/PacketType.h"

#include <QDataStream>
#include <QDebug>
#include <QHostAddress>

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
                << socket->peerAddress().toString()
                << ":" << socket->peerPort();
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
    while (Protocol::tryExtractPacket(it.value().buffer, packet))
        handlePacket(socket, packet);
}

void Server::onDisconnected()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    auto it = m_clients.find(socket);
    if (it != m_clients.end()) {
        const QString roomName = it.value().roomName;
        if (!roomName.isEmpty() && m_rooms.contains(roomName)) {
            m_rooms[roomName].clients.removeAll(socket);
            qInfo() << "[Server] клиент вышел из комнаты" << roomName
                    << "; осталось:" << m_rooms[roomName].clients.size();

            if (m_rooms[roomName].clients.isEmpty()) {
                qInfo() << "[Server] комната" << roomName << "пуста — удаляем";
                m_rooms.remove(roomName);
            }
        }
        m_clients.erase(it);
    }

    qInfo() << "[Server] клиент отключился";
    socket->deleteLater();
}

void Server::handlePacket(QTcpSocket *socket, const Protocol::Packet &packet)
{
    auto &state = m_clients[socket];

    switch (packet.type) {

    case PacketType::Join: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QString roomName;
        in >> roomName;
        if (roomName.isEmpty()) roomName = QStringLiteral("default");

        state.roomName = roomName;
        Room &room = m_rooms[roomName];
        if (!room.clients.contains(socket))
            room.clients.append(socket);

        qInfo() << "[Server]" << socket->peerAddress().toString()
                << "вошёл в комнату" << roomName
                << "(всего:" << room.clients.size() << ")";

        sendSnapshot(socket, roomName);
        break;
    }

    case PacketType::Draw: {
        if (state.roomName.isEmpty()) return;
        auto obj = Serializer::deserializeObject(packet.payload);
        if (obj) {
            bool replaced = false;
            auto &objs = m_rooms[state.roomName].objects;
            for (int i = 0; i < objs.size(); ++i) {
                if (objs[i]->uuid == obj->uuid) {
                    objs[i] = obj;
                    replaced = true;
                    break;
                }
            }
            if (!replaced) objs.append(obj);
        }
        QByteArray frame = Protocol::makePacket(PacketType::Draw, packet.payload);
        broadcast(state.roomName, frame, socket);
        break;
    }

    case PacketType::Erase: {
        if (state.roomName.isEmpty()) return;
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; in >> uuid;

        auto &objs = m_rooms[state.roomName].objects;
        for (int i = 0; i < objs.size(); ++i)
            if (objs[i]->uuid == uuid) { objs.removeAt(i); break; }

        QByteArray frame = Protocol::makePacket(PacketType::Erase, packet.payload);
        broadcast(state.roomName, frame, socket);
        break;
    }

    case PacketType::Fill: {
        if (state.roomName.isEmpty()) return;
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; QColor color;
        in >> uuid >> color;

        for (auto &obj : m_rooms[state.roomName].objects) {
            if (obj->uuid == uuid) {
                obj->filled    = true;
                obj->fillColor = color;
                break;
            }
        }

        QByteArray frame = Protocol::makePacket(PacketType::Fill, packet.payload);
        broadcast(state.roomName, frame, socket);
        break;
    }

    case PacketType::Move: {
        if (state.roomName.isEmpty()) return;
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; QPointF delta;
        in >> uuid >> delta;

        for (auto &obj : m_rooms[state.roomName].objects) {
            if (obj->uuid != uuid) continue;
            switch (obj->type) {
            case ObjectType::Stroke:
                static_cast<StrokeObject*>(obj.get())->path.translate(delta);
                break;
            case ObjectType::Rect:
                static_cast<RectObject*>(obj.get())->rect.translate(delta);
                break;
            case ObjectType::Ellipse:
                static_cast<EllipseObject*>(obj.get())->rect.translate(delta);
                break;
            case ObjectType::Triangle: {
                auto *t = static_cast<TriangleObject*>(obj.get());
                t->p1 += delta; t->p2 += delta; t->p3 += delta;
                break;
            }
            }
            break;
        }

        QByteArray frame = Protocol::makePacket(PacketType::Move, packet.payload);
        broadcast(state.roomName, frame, socket);
        break;
    }

    case PacketType::Clear: {
        if (state.roomName.isEmpty()) return;
        m_rooms[state.roomName].objects.clear();
        QByteArray frame = Protocol::makePacket(PacketType::Clear, packet.payload);
        broadcast(state.roomName, frame, socket);
        break;
    }

    case PacketType::Ping: {
        socket->write(Protocol::makePacket(PacketType::Pong, QByteArray()));
        break;
    }

    default:
        qDebug() << "[Server] неизвестный пакет:" << quint8(packet.type);
        break;
    }
}

void Server::sendSnapshot(QTcpSocket *socket, const QString &roomName)
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    const Room &room = m_rooms[roomName];
    out << qint32(room.objects.size());
    for (const auto &obj : room.objects)
        out << Serializer::serializeObject(*obj);

    socket->write(Protocol::makePacket(PacketType::Snapshot, payload));
    qInfo() << "[Server] снэпшот отправлен, объектов:" << room.objects.size();
}

void Server::broadcast(const QString &roomName,
                       const QByteArray &frame,
                       QTcpSocket *exclude)
{
    if (!m_rooms.contains(roomName)) return;
    for (QTcpSocket *client : m_rooms[roomName].clients) {
        if (client == exclude) continue;
        if (client->state() != QAbstractSocket::ConnectedState) continue;
        client->write(frame);
    }
}
