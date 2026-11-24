#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "wrapper/downloadPanel/downloadPanelWrap.h"

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Material");

    QGuiApplication app(argc, argv);
    app.setApplicationName("YeetDownloader");
    app.setOrganizationName("YeetDownloader");

    // Created before the engine so it outlives the QML bindings that read it.
    downloadPanelWrap downloadPanel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("downloadPanel", &downloadPanel);

    engine.loadFromModule("YeetDownloader", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
