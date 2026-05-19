#include "chatserver.hpp"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

Server::Server(quint16 port, QObject *parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection,
            this, &Server::onNewConnection);

    if (!m_server.listen(QHostAddress::Any, port)) {
        qCritical() << "Server failed to start:" << m_server.errorString();
        return;
    }
    qInfo() << "Server listening on port" << port;
}

// here is ok

void Server::onNewConnection() {
    while (m_server.hasPendingConnections()) {
        QTcpSocket *socket = m_server.nextPendingConnection();
        m_clients.insert(socket);

        connect(socket, &QTcpSocket::readyRead,
                this, &Server::onReadyRead);
        connect(socket, &QTcpSocket::disconnected,
                this, &Server::onDisconnected);

        qInfo() << "Client connected:" << socket->peerAddress().toString();
    }
}
// 


void Server::onReadyRead() {
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    while (socket->canReadLine()) {
        QByteArray data = socket->readAll();
        qDebug() << "Received:" << data;

        QJsonParseError err;
        
        QJsonDocument::fromJson(data, &err);

        if (err.error != QJsonParseError::NoError) {
            qWarning() << "Bad JSON from client, dropping.";
            continue;
        }

        broadcast(data, socket);
    }
}

void Server::onDisconnected() {
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    m_clients.remove(socket);
    qInfo() << "Client disconnected.";

    // Notify others
    QJsonObject leave;
    leave["type"]   = "system";
    leave["text"]   = "A user left the chat.";
    broadcast(QJsonDocument(leave).toJson(QJsonDocument::Compact) + "\n");

    socket->deleteLater();
}

void Server::broadcast(const QByteArray &data, QTcpSocket *exclude) {
    for (QTcpSocket *client : std::as_const(m_clients)) {
        if (client != exclude && client->state() == QAbstractSocket::ConnectedState)
            client->write(data);
    }
}