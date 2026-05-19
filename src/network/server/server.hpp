#pragma once
#include <QObject>
#include <QTcpServer>
#include <QSet>

class QTcpSocket;

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(quint16 port, QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void broadcast(const QByteArray &data, QTcpSocket *exclude = nullptr);

    QTcpServer          m_server;
    QSet<QTcpSocket*>   m_clients;
};