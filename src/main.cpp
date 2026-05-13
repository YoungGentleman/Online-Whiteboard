#include <QApplication>
#include <QStyleFactory>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Whiteboard");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("WhiteboardTeam");

    // Enable high-DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough
    );

    MainWindow window;
    window.show();

    return app.exec();
}
