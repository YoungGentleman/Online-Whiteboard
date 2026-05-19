#include <QCoreApplication>
#include "server.hpp"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    Server server(12345);
    return app.exec();
}
