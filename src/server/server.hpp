#pragma once
#include <QObject>
#include <QTcpServer>
#include <QSet>
#include <memory>
#include <QtConcurrent>
#include <QThread>

class QTcpSocket;

struct Room{                                                                    // rooms will be processed in peculiar thread
    QVector<std::shared_ptr<DrawObject>> m_figures;                             // all the user's items they will be uploaded as user connect
    QSet<QTcpSocket*>   m_clients;                                              // all the users in a room
};


class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(quint16 port, QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void mergeChanges(QByteArray& exclude_data, QTcpSocket *exclude = nullptr); 
    void broadcast(const QByteArray& data, QTcpSocket *exclude = nullptr);
    QVector<Room> Rooms;                                                         // all the rooms on the server
    QTcpServer  m_server;
};