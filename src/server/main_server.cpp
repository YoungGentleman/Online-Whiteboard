#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QMetaType>
#include <memory>

#include "Server.h"
#include "../data/DrawObject.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<std::shared_ptr<DrawObject>>("std::shared_ptr<DrawObject>");
    qRegisterMetaType<QUuid>("QUuid");
    app.setApplicationName("whiteboard_server");
    app.setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Whiteboard collaboration server");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("port", "Порт для прослушивания (по умолчанию 45000)");
    parser.process(app);

    quint16 port = 45000;
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        bool ok = false;
        const int v = args.first().toInt(&ok);
        if (ok && v > 0 && v < 65536) {
            port = static_cast<quint16>(v);
        } else {
            qCritical() << "Некорректный порт:" << args.first();
            return 1;
        }
    }

    Server server(port);
    if (!server.isListening()) {
        qCritical() << "Сервер не смог стартовать";
        return 1;
    }

    return app.exec();
}