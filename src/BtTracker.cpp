#include <BtTracker.h>
#include <QCryptographicHash>
#include <QUrlQuery>

using namespace BtQt;

/* Get the last 'e' of a bencoded list or dictionary, implemented in BtTorrent */
extern int getLastE(QByteArray const &data, int pos);

static QByteArray infoInMetadata(QByteArray const &torrentMetadata)
{
    int infoIdx = torrentMetadata.indexOf("4:infod");
    if(infoIdx == -1) {
        throw -1;
    }

    infoIdx += 6;
    int lastE = getLastE(torrentMetadata, infoIdx);

    return torrentMetadata.mid(infoIdx, lastE - infoIdx + 1);
}

void BtQt::torrentInfoHash(QFile &torrentFile, QByteArray &ret)
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

    ret = QCryptographicHash::hash(info, QCryptographicHash::Sha1);
}

BtTrackerRequest::BtTrackerRequest() : event(BtTrackerDownloadEvent::empty),
    requestDataGenerated(false)
{

}

BtTrackerRequest::BtTrackerRequest(QByteArray const &info_hash,
        QByteArray const &peer_id, quint16 port, quint64 uploaded,
        quint64 downloaded, quint64 left, QHostAddress const &ip,
        BtTrackerDownloadEvent event) :
    info_hash(info_hash), peer_id(peer_id), ip(ip), port(port),
    uploaded(uploaded), downloaded(downloaded), left(left), event(event),
    requestDataGenerated(false)
{

}

void BtTrackerRequest::setInfoHash(QByteArray const &info_hash)
{
    requestDataGenerated = false;
    this->info_hash = info_hash;
}

void BtTrackerRequest::setPeerId(QByteArray const &peer_id)
{
    requestDataGenerated = false;
    this->peer_id = peer_id;
}

void BtTrackerRequest::setIp(QHostAddress const &ip)
{
    requestDataGenerated = false;
    this->ip = ip;
}

void BtTrackerRequest::setPort(quint16 port)
{
    requestDataGenerated = false;
    this->port = port;
}

void BtTrackerRequest::setUploaded(quint64 uploaded)
{
    requestDataGenerated = false;
    this->uploaded = uploaded;
}

void BtTrackerRequest::setDownloaded(quint64 downloaded)
{
    requestDataGenerated = false;
    this->downloaded = downloaded;
}

void BtTrackerRequest::setLeft(quint64 left)
{
    requestDataGenerated = false;
    this->left = left;
}

void BtTrackerRequest::setEvent(BtTrackerDownloadEvent event)
{
    requestDataGenerated = false;
    this->event = event;
}

QByteArray BtTrackerRequest::getInfoHash()
{
    if(info_hash.isEmpty()) {
        qDebug() << "Requesting an empty info hash!";
    }
    return info_hash;
}

QByteArray BtTrackerRequest::getPeerId()
{
    if(peer_id.isEmpty()) {
        qDebug() << "Requesting an empty peer id!";
    }
    return peer_id;
}

QHostAddress BtTrackerRequest::getIp()
{
    return ip;
}

quint16 BtTrackerRequest::getPort()
{
    return port;
}

#ifndef QT_NO_DEBUG
void BtTrackerRequest::display()
{
    qDebug() << "info_hash: " << info_hash.toHex();
    qDebug() << "peer_id: " << peer_id;
    if(!ip.isNull()) qDebug() << "listening: " << ip << ":" << port;
    else qDebug() << "listening: " << port;
    qDebug() << "uploaded: " << uploaded;
    qDebug() << "downloaded: " << downloaded;
    qDebug() << "left: " << left;
    qDebug() << "event" << (int)event;
}
#endif // QT_NO_DEBUG

static QString urlencodeUnicode(QByteArray const &in)
{
    QString ret;
    for (auto i : in) {
        if(i >= '0' && i <= '9') ret.append(i);
        else if(i >= 'a' && i <= 'z') ret.append(i);
        else if(i >= 'A' && i <= 'z') ret.append(i);
        else if(i == '.' || i == '-' || i == '_' || i =='~') ret.append(i);
        /* Encode \xss to "%nn", "nn" is the hexadecimal value of the byte */
        else {
            ret.append("%");
            ret.append(QByteArray(1, i).toHex().toUpper());
        }
    }

    return ret;
}

const QByteArray& BtTrackerRequest::toRequestData()
{
    /* If not generated or out of date */
    if(!requestDataGenerated) {
        requestData.clear();
        /* generate */
        QUrlQuery params;
        params.addQueryItem("info_hash", urlencodeUnicode(info_hash));
        params.addQueryItem("peer_id", urlencodeUnicode(peer_id));
        params.addQueryItem("port", QByteArray::number(port));
        params.addQueryItem("uploaded", QByteArray::number(uploaded));
        params.addQueryItem("downloaded", QByteArray::number(downloaded));
        params.addQueryItem("left", QByteArray::number(left));

        if(!ip.isNull()) params.addQueryItem("ip", ip.toString());
        switch(event) {
            case BtTrackerDownloadEvent::started:
                params.addQueryItem("event", "started");
                break;
            case BtTrackerDownloadEvent::completed:
                params.addQueryItem("event", "completed");
                break;
            case BtTrackerDownloadEvent::stopped:
                params.addQueryItem("event", "stopped");
                break;
            default:
            /* same as empty */
                break;
        }
        qDebug() << urlencodeUnicode(info_hash);
        qDebug() << params.query(QUrl::EncodeUnicode);
        requestData.append(params.query(QUrl::EncodeUnicode));
        qDebug() << requestData;
        requestDataGenerated = true;
    }

    return requestData;
}
