#include "BtTorrent.h"
#include <QByteArray>
#include <QIODevice>
#include <QDebug>

using namespace BtQt;

bool BtTorrent::decodeTorrentFile(QFile &torrentFile)
{
    if(!torrentFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file " << torrentFile.fileName() << " in read-only mode.";
        throw -1;
    }

    QByteArray torrentData = torrentFile.readAll();
    torrentFile.close();

    try {
        BtQt::BtDecodeBencodeDictionary(QString::fromUtf8(torrentData), torrentObject);
    } catch (std::exception e) {
        qDebug() << "Cannot decode torrent file " << torrentFile.fileName() << "!";
        return false;
    }

    return true;
}
