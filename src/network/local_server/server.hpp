#pragma once
#include <QObject>
#include <QTcpServer>
#include <QSet>
#include <memory>
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
    QVector<std::shared_ptr<DrawObject>> objects_on_server;
    QTcpServer          m_server;
    QSet<QTcpSocket*>   m_clients;
};