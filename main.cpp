#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "downloader.h"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Material");

    QGuiApplication app(argc, argv);
    app.setApplicationName("YeetDownloader");
    app.setOrganizationName("YeetDownloader");

    QQmlApplicationEngine engine;

    Downloader downloader;
    engine.rootContext()->setContextProperty("downloader", &downloader);

    // Module name updated here
    engine.loadFromModule("YeetDownloader", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}