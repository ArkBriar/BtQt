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

    torrentInfoHash(metadata, ret);
}

void BtQt::torrentInfoHash(QByteArray const &torrentMetadata, QByteArray &ret)
{
    QByteArray info;
    try {
        info = infoInMetadata(torrentMetadata);
    } catch (int e) {
        qDebug() << "Can not find \'info\' section in torrent metadata. Torrent is broken.";
        return;
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
    quint16 port = trackerUrl.port(80);
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

    QByteArray string;
    if(trackerUrl.hasQuery()) {
        string = "GET " + trackerUrl.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority) + '&' + req.toRequestData() + " HTTP/1.1\r\n";
    } else {
        string = "GET " + trackerUrl.toEncoded(QUrl::RemoveScheme | QUrl::RemoveAuthority) + '?' + req.toRequestData() + " HTTP/1.1\r\n";
    }

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

QMap<QString, QVariant> BtQt::parseTrackerResponse(QByteArray const &response)
{
    QMap<QString, QVariant> ret;

    /* This bencoded dictionary is weired.
     * Shit, the peers will be decoded specially
     * */
    int peersIdx = response.indexOf("5:peers");
    if(peersIdx != -1)
        BtDecodeBencodeDictionary(response.mid(0, peersIdx).append('e'), ret);
    else {
        BtDecodeBencodeDictionary(response, ret);
        return ret;
    }

    QByteArray peers;
    peers.append(response.mid(peersIdx + 7, response.size() - peersIdx - 8));

    QMap<QString, QVariant> peer;
    QList<QVariant> peerList;
    bool haveId = false;
    if(peers.at(0) == '7') haveId = true;
    for(auto i = 0; peers.at(i) != 'e'; ) {
        if(haveId == true) {
            if(peers.at(i) == '7') {
                /* peer id */
                i += 7 + 2;
                /* id is string, 20 */
                i += 3;
                peer.insert("peer id", peers.mid(i, 20));
                i += 20;
            } else {
                qDebug() << "There's shit in trackers response";
                throw -1;
            }
        }
        if(peers.at(i) == '2') {
            /* ip */
            i += 2 + 2;
            int colonIdx = peers.indexOf(':', i);
            int ipLen = peers.mid(i, colonIdx - i).toInt();
            i = colonIdx + 1;
            peer.insert("ip", peers.mid(i, ipLen));
            i += ipLen;
        } else {
            qDebug() << "There's shit in trackers response";
            throw -1;
        }

        if(peers.at(i) == '4') {
            /* port */
            i += 4 + 2;
            int eIdx = peers.indexOf('e', i);
            peer.insert("port", peers.mid(i + 1, eIdx - i - 1));
            i = eIdx + 1;
        } else {
            qDebug() << "There's shit in trackers response";
            throw -1;
        }
        peerList.append(peer);
    }

    ret.insert("peers", peerList);

    return ret;
}

BtTrackerResponse::BtTrackerResponse(QMap<QString, QVariant> const &response)
    : Interval(-1), Complete(-1), InComplete(-1), MinInterval(-1)
{
    /* Check if failed */
    if(response.contains("failure reason")) {
        failureReason = response.value("failure reason").toString();
    } else {
        if(response.contains("warning message")) {
            warningMessage = response.value("warning message").toString();
        }

        if(response.contains("interval")) {
            Interval = response.value("interval").toInt();
        } else {
            qDebug() << "Warning: There's no interval in tracker's response";
            Interval = -1;
        }

        if(response.contains("min interval")) {
            MinInterval = response.value("min interval").toInt();
        } else {
            MinInterval = -1;
        }

        if(response.contains("tracker id")) {
            TrackerId = response.value("tracker id").toByteArray();
        }

        if(response.contains("complete")) {
            Complete = response.value("complete").toInt();
        } else {
            Complete = -1;
        }

        if(response.contains("incomplete")) {
            InComplete = response.value("incomplete").toInt();
        } else {
            InComplete = -1;
        }

        if(response.contains("peers")) {
            if(response.value("peers").canConvert(QMetaType::QVariantList)) {
                /* Dictionary model */
                auto _Peers = response.value("peers").toList();
                for(auto i : _Peers) {
                    Peers.append(i.toMap());
                }
            } else {
                /* Binary model */
                auto _Peers = response.value("peers").toByteArray();
                QMap<QString, QVariant> _Peer;
                for (auto i = 0; i < _Peers.size(); i += 6) {
                    _Peer.insert("ip", _Peers.at(i) + '.' + _Peers.at(i + 1) + '.'                            + _Peers.at(i + 2) + '.' + _Peers.at(i + 3));
                    _Peer.insert("port", _Peers.mid(i + 4, 2).toInt());
                    Peers.append(_Peer);
                }
            }
        }
    }
}

int BtTrackerResponse::interval() const
{
    return Interval;
}

QByteArray BtTrackerResponse::trackerId() const
{
    return TrackerId;
}

int BtTrackerResponse::complete() const
{
    return Complete;
}

int BtTrackerResponse::incomplete() const
{
    return InComplete;
}

auto BtTrackerResponse::peers()->decltype(Peers) const
{
    return Peers;
}

int BtTrackerResponse::minInterval() const
{
    return MinInterval;
}

bool BtTrackerResponse::failed(QString &reason) const
{
    if(failureReason.isEmpty()) return false;
    else if(Peers.isEmpty()) {
        reason = failureReason;
    }
    return true;
}

bool BtTrackerResponse::warned(QString &warning) const
{
    if(warningMessage.isEmpty()) return false;
    warning = warningMessage;
    return true;
}

#ifndef QT_NO_DEBUG
void BtTrackerResponse::display() const
{
    qDebug() << "failure reason: " << failureReason;
    qDebug() << "warning message: " << warningMessage;

    qDebug() << "interval: " << Interval;
    qDebug() << "tracker id: " << TrackerId;
    qDebug() << "complete: " << Complete;
    qDebug() << "incomplete: " << InComplete;

    qDebug() << "peers: " << Peers;
    /*
     *for (auto i : Peers) {
     *    qDebug() << "ip: " << i.value("ip").toByteArray();
     *    qDebug() << "port: " << i.value("port").toInt();
     *}
     */

    qDebug() << "min interval: " << MinInterval;
}
#endif // QT_NO_DEBUG
