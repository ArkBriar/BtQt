#include <BtTorrent.h>
#include <QDebug>

int main()
{
    BtQt::BtTorrent t;
    QFile file("../test/test.torrent");
    qDebug() << "Decode start...";
    if(t.decodeTorrentFile(file)) {
        qDebug() << "Decode succeed!";
    } else {
        qDebug() << "Decode failed!";
    }
    t.display();

    return 0;
}
