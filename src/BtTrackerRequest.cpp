#include <BtQt.h>
#include <QCryptographicHash>
#include <QUrlQuery>
#include <QTcpSocket>
#include <QAbstractSocket>

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
    } catch (int e) {
        qDebug() << "Can not find \'info\' section in torrent metadata. Torrent is broken.";
        throw -1;
    }

    ret = QCryptographicHash::hash(info, QCryptographicHash::Sha1);
}

BtTrackerRequest::BtTrackerRequest() : event(BtTrackerDownloadEvent::empty),
    compact(false), no_peer_id(false), numwant(50), requestDataGenerated(false)
{

}

BtTrackerRequest::BtTrackerRequest(QByteArray const &info_hash,
        QByteArray const &peer_id, quint16 port, quint64 uploaded,
        quint64 downloaded, quint64 left, QHostAddress const &ip,
        BtTrackerDownloadEvent event) :
    info_hash(info_hash), peer_id(peer_id), ip(ip), port(port),
    uploaded(uploaded), downloaded(downloaded), left(left), event(event),
    compact(false), no_peer_id(false), numwant(50),
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
    if(port == this->port) return;

    requestDataGenerated = false;
    this->port = port;
}

void BtTrackerRequest::setUploaded(quint64 uploaded)
{
    if(uploaded == this->uploaded) return;

    requestDataGenerated = false;
    this->uploaded = uploaded;
}

void BtTrackerRequest::setDownloaded(quint64 downloaded)
{
    if(downloaded == this->downloaded) return;

    requestDataGenerated = false;
    this->downloaded = downloaded;
}

void BtTrackerRequest::setLeft(quint64 left)
{
    if(left == this->left) return;

    requestDataGenerated = false;
    this->left = left;
}

void BtTrackerRequest::setEvent(BtTrackerDownloadEvent event)
{
    if(event == this->event) return;

    requestDataGenerated = false;
    this->event = event;
}

void BtTrackerRequest::setCompact(bool compact)
{
    if(compact == this->compact) return;

    requestDataGenerated = false;
    this->compact = compact;
}

void BtTrackerRequest::setNoPeerId(bool no_peer_id)
{
    if(no_peer_id == this->no_peer_id) return;

    requestDataGenerated = false;
    this->no_peer_id = no_peer_id;
}

void BtTrackerRequest::setNumwant(int numwant)
{
    if(numwant == this->numwant) return;

    requestDataGenerated = false;
    this->numwant = numwant;
}

QByteArray BtTrackerRequest::getInfoHash() const
{
    if(info_hash.isEmpty()) {
        qDebug() << "Requesting an empty info hash!";
    }
    return info_hash;
}

QByteArray BtTrackerRequest::getPeerId() const
{
    if(peer_id.isEmpty()) {
        qDebug() << "Requesting an empty peer id!";
    }
    return peer_id;
}

QHostAddress BtTrackerRequest::getIp() const
{
    return ip;
}

quint16 BtTrackerRequest::getPort() const
{
    return port;
}

bool BtTrackerRequest::isCompact() const
{
    return compact;
}

bool BtTrackerRequest::isNoPeerId() const
{
    return no_peer_id;
}

int BtTrackerRequest::getNumwant() const
{
    return numwant;
}

#ifndef QT_NO_DEBUG
void BtTrackerRequest::display() const
{
    qDebug() << "info_hash: " << info_hash.toHex();
    qDebug() << "peer_id: " << peer_id;
    if(!ip.isNull()) qDebug() << "listening: " << ip << ":" << port;
    else qDebug() << "listening: " << port;
    qDebug() << "uploaded: " << uploaded;
    qDebug() << "downloaded: " << downloaded;
    qDebug() << "left: " << left;
    qDebug() << "event" << (int)event;

    qDebug() << "compact: " << compact;
    qDebug() << "no_peer_id: " << no_peer_id;
    qDebug() << "numwant: " << numwant;
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

const QByteArray& BtTrackerRequest::toRequestData() const
{
    /* If not generated or out of date */
    if(!requestDataGenerated) {
        const_cast<QByteArray &>(requestData).clear();
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

        if(compact) params.addQueryItem("compact", compact?"1":"0");
        else if(no_peer_id) params.addQueryItem("no_peer_id", no_peer_id?"1":"0");
        if(numwant != 50) params.addQueryItem("numwant", QByteArray::number(numwant));

        const_cast<QByteArray &>(requestData).append(params.query(QUrl::EncodeUnicode));
        const_cast<bool &>(requestDataGenerated) = true;
    }

    return requestData;
}

QByteArray BtQt::sendTrackerRequest(BtTrackerRequest const &req, QUrl trackerUrl)
{
    /* For the reason that QUrl has used RFC3986 instead of RFC 1738,
     * I have to emulate an HTTP GET request using tcp socket. */
    QTcpSocket socket;
    QString host = trackerUrl.host();
    quint16 port = trackerUrl.port();
#ifndef QT_NO_DEBUG
    qDebug() << "Host: " << host;
    qDebug() << "Port: " << port;
#endif // QT_NO_DEBUG
    /*
     *QString host = trackerUrl.toEncoded(QUrl::RemoveScheme | QUrl::RemovePath
     *        | QUrl::RemoveAuthority);
     */
    socket.connectToHost(host, port);
    if(!socket.waitForConnected(1000)) {
        qDebug() << "Can not establish tcp connection to"  << host + ":" + QString::number(port);
        throw -1;
    }
    socket.setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    /* HTTP 1.1 header, for more information please go to RFC2616 */
    QByteArray header;
    header.append("HOST: " + host + ":" + QString::number(port) + "\r\n");
    header.append("User-Agent: " + BtQt::application + " " + BtQt::version + "\r\n");
    header.append("Accept: */*\r\n");
    header.append("Connection: Keep-Alive\r\n");
    header.append("\r\n");

    QByteArray string = "GET " + trackerUrl.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority) + "?" + req.toRequestData() + " HTTP/1.1\r\n";

#ifndef QT_NO_DEBUG
    qDebug() << "Header: " << header;
    qDebug() << "String: " << string;
#endif // QT_NO_DEBUG

    socket.write(string + header);

    if(!socket.waitForReadyRead(1000)) {
        qDebug() << "There were some error occured or possibly time out! Can not get reply!";
        throw -1;
    }
    QByteArray trackerReply = socket.readAll();
    if(trackerReply.isEmpty()) {
        qDebug() << "Warnning! We got an empty reply!";
    }

    /* Get the reply data */
    int replyIdx = trackerReply.indexOf("\r\n\r\n") + 4;

    return trackerReply.mid(replyIdx);
}

