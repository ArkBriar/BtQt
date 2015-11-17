#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>

#include <BtTorrent.h>

#ifndef QT_NO_DEBUG
#define DebugQmlSrc(qmlfile) "../qml/"#qmlfile
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
#ifdef QT_NO_DEBUG
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
#else
    engine.load(QUrl(QUrl::fromLocalFile(DebugQmlSrc(main.qml))));
#endif

    BtQt::BtTorrent t;
    QFile file("../test/test.torrent");
    qDebug() << "Decode start...";
    if(t.decodeTorrentFile(file)) {
        qDebug() << "Decode succeed!";
    } else {
        qDebug() << "Decode failed!";
    }
    t.display();

    return app.exec();
}
