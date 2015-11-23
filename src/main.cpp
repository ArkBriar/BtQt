#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>

#include <BtTorrent.h>

#ifndef QT_NO_DEBUG
#define DebugQmlSrc(qmlfile) "../ui/qml/"#qmlfile
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
#ifdef QT_NO_DEBUG
    engine.load(QUrl(QStringLiteral("qrc:qml/main.qml")));
#else
    engine.load(QUrl(QUrl::fromLocalFile(DebugQmlSrc(main.qml))));
#endif

    return app.exec();
}
