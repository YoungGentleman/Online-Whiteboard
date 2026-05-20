#include "NetworkManager.h"
#include "../data/DrawObject.h"

#include <QDataStream>
#include <QIODevice>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    m_clock.resize(1);

    connect(&m_socket, &QTcpSocket::connected,
            this, &NetworkManager::onConnected);
    connect(&m_socket, &QTcpSocket::disconnected,
            this, &NetworkManager::onDisconnected);
    connect(&m_socket, &QTcpSocket::readyRead,
            this, &NetworkManager::onReadyRead);
    connect(&m_socket, &QAbstractSocket::errorOccurred,
            this, &NetworkManager::onErrorOccurred);

    m_handshakeTimer.setSingleShot(true);
    connect(&m_handshakeTimer, &QTimer::timeout,
            this, &NetworkManager::onHandshakeTimeout);
}

void NetworkManager::connectToServer(const QString &host, quint16 port,
                                     const QString &roomName)
{
    m_roomName = roomName.isEmpty() ? QStringLiteral("default") : roomName;
    m_buffer.clear();
    m_handshakeDone = false;
    m_handshakeTimer.stop();
    m_socket.abort();
    m_socket.connectToHost(host, port);
}

void NetworkManager::disconnectFromServer()
{
    m_handshakeTimer.stop();
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
        m_socket.disconnectFromHost();
}

bool NetworkManager::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

void NetworkManager::sendDraw(const std::shared_ptr<DrawObject> &obj)
{
    if (!obj || !isConnected()) return;
    sendPacket(PacketType::Draw, Serializer::serializeObject(*obj));
}

void NetworkManager::sendErase(const QUuid &uuid)
{
    if (!isConnected()) return;
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << uuid;
    sendPacket(PacketType::Erase, payload);
}

void NetworkManager::sendFill(const QUuid &uuid, const QColor &color)
{
    if (!isConnected()) return;
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << uuid << color;
    sendPacket(PacketType::Fill, payload);
}

void NetworkManager::sendMove(const QUuid &uuid, const QPointF &delta)
{
    if (!isConnected()) return;
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << uuid << delta;
    sendPacket(PacketType::Move, payload);
}

void NetworkManager::sendClear()
{
    if (!isConnected()) return;
    sendPacket(PacketType::Clear, QByteArray());
}

void NetworkManager::sendPacket(PacketType type, const QByteArray &payload)
{
    m_clock.send(m_nodeId);
    m_socket.write(Protocol::makePacket(type, m_clock, payload));
}

void NetworkManager::onConnected()
{
    QByteArray payload;
    QDataStream out(&payload, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << m_roomName;
    sendPacket(PacketType::Join, payload);
    m_handshakeDone = false;
    m_handshakeTimer.start(5000);
}

void NetworkManager::onHandshakeTimeout()
{
    if (m_handshakeDone) return;
    m_socket.abort();
    emit connectionError(
        "Сервер не отвечает. Возможно, по указанному адресу нет "
        "Whiteboard-сервера или он работает на другом порту.");
}

void NetworkManager::onDisconnected()
{
    emit disconnectedFromServer();
}

void NetworkManager::onErrorOccurred(QAbstractSocket::SocketError)
{
    emit connectionError(m_socket.errorString());
}

void NetworkManager::onReadyRead()
{
    m_buffer.append(m_socket.readAll());
    Protocol::Packet packet;
    while (Protocol::tryExtractPacket(m_buffer, packet))
        handlePacket(packet);
}

void NetworkManager::handlePacket(const Protocol::Packet &packet)
{
    // правило Лэмпорта при получении
    m_clock.receive(packet.clock, m_nodeId);

    if (!m_handshakeDone) {
        m_handshakeDone = true;
        m_handshakeTimer.stop();
        emit connectedToServer();
    }

    switch (packet.type) {

    case PacketType::Snapshot: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        qint32 count = 0;
        in >> count;
        QVector<std::shared_ptr<DrawObject>> objects;
        objects.reserve(count);
        for (qint32 i = 0; i < count; ++i) {
            QByteArray objData;
            in >> objData;
            auto obj = Serializer::deserializeObject(objData);
            if (obj) objects.append(obj);
        }
        emit snapshotReceived(objects);
        break;
    }

    case PacketType::Draw: {
        auto obj = Serializer::deserializeObject(packet.payload);
        if (obj) emit remoteDrawReceived(obj);
        break;
    }

    case PacketType::Erase: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; in >> uuid;
        emit remoteEraseReceived(uuid);
        break;
    }

    case PacketType::Fill: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; QColor color;
        in >> uuid >> color;
        emit remoteFillReceived(uuid, color);
        break;
    }

    case PacketType::Move: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        QUuid uuid; QPointF delta;
        in >> uuid >> delta;
        emit remoteMoveReceived(uuid, delta);
        break;
    }

    case PacketType::Clear:
        emit remoteClearReceived();
        break;

    case PacketType::UserJoined: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        qint32 count = 0; in >> count;
        emit userJoined(int(count));
        break;
    }

    case PacketType::UserLeft: {
        QDataStream in(packet.payload);
        in.setVersion(QDataStream::Qt_6_0);
        qint32 count = 0; in >> count;
        emit userLeft(int(count));
        break;
    }

    default:
        break;
    }
}
