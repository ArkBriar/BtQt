#include "BtTracker.h"
#include <QCryptographicHash>

using namespace BtQt;

extern int getLastE(QByteArray const &data, int pos);

static QByteArray infoInMetadata(QByteArray const &torrentMetadata)
{
    int infoIdx = torrentMetadata.indexOf("4:infod");
    if(infoIdx == -1) {
        throw -1;
    }

    int lastE = getLastE(torrentMetadata, infoIdx + 6);

    return torrentMetadata.mid(infoIdx, lastE - infoIdx + 1);
}

void BtTracker::torrentInfoHash(QFile torrentFile)
{
    if(!torrentFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Can not open torrent file " << torrentFile.fileName() << " in read-only mode.";
        throw -1;
    }
    QByteArray metadata = torrentFile.readAll();
    torrentFile.close();

    QByteArray info;
    try {
        info = infoInMetadata(metadata);
    } catch (std::exception e) {
        qDebug() << "Can not find \'info\' section in torrent metadata. Torrent is broken.";
        throw -1;
    }

    info_hash = QCryptographicHash::hash(info, QCryptographicHash::Sha1);
}

void BtTracker::BencodeEncode(QByteArray &encoded)
{
    /* Not implemented */
    encoded.isNull();
}
