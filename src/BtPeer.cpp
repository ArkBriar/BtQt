#include <BtQt.h>
#include <QTcpSocket>

using namespace BtQt;

QByteArray BtQt::generatePeerId()
{
    QByteArray peer_id = "-";

    if(AzureusClientId.size() != 2) peer_id.append(AzureusClientId.mid(0, 2));
    else peer_id.append(AzureusClientId);
    if(versionInString.size() != 4) peer_id.append(versionInString.mid(0, 4));
    else peer_id.append(versionInString);
    peer_id.append('-');

    for(auto i = 0; i < 12; ++ i) {
        peer_id.append(QByteArray::number(qrand() % 10));
    }

    Q_ASSERT(peer_id.size() == 20);

    return peer_id;
}

BtPeer::BtPeer(BtTorrent const &torrentRef)
    : AmChoking(false), AmInterested(false), PeerChoking(false),
    PeerInterested(false), torrentRef(torrentRef)
{
    pieces.resize(torrentRef.pieceLength());
}

BtPeer::BtPeer(BtTorrent const &torrentRef, QByteArray const &peer_id,
        QHostAddress const &ip, quint16 port)
    : peer_id(peer_id), peerIp(ip), peerPort(port), AmChoking(false),
    AmInterested(false), PeerChoking(false), PeerInterested(false),
    torrentRef(torrentRef)
{
    pieces.resize(torrentRef.pieceLength());
}

void BtPeer::setPeerId(QByteArray const &peer_id)
{
    this->peer_id = peer_id;
}

void BtPeer::setIp(QHostAddress const &ip)
{
    peerIp = ip;
}

void BtPeer::setPort(quint16 port)
{
    peerPort = port;
}

void BtPeer::setTorrentRef(BtTorrent const &torrentRef)
{
    const_cast<BtTorrent &>(this->torrentRef) = torrentRef;
}

void BtPeer::pieceOk(int idx)
{
    pieces.setBit(idx);
}

QByteArray BtPeer::getPeerId() const
{
    return peer_id;
}

QHostAddress BtPeer::getIp() const
{
    return peerIp;
}

quint16 BtPeer::getPort() const
{
    return peerPort;
}

const BtTorrent& BtPeer::getTorrentRef() const
{
    return torrentRef;
}

bool BtPeer::amChoking() const
{
    return AmChoking;
}

bool BtPeer::amInterested() const
{
    return AmInterested;
}

bool BtPeer::peerChoking() const
{
    return PeerChoking;
}

bool BtPeer::peerInterested() const
{
    return PeerInterested;
}

QBitArray BtPeer::getPieceBitArray() const
{
    return pieces;
}

bool BtPeer::havePiece(int idx) const
{
    return pieces.testBit(idx);
}

BtRemotePeer::BtRemotePeer(BtTorrent const &torrentRef) :
    BtPeer(torrentRef)
{

}

BtRemotePeer::BtRemotePeer(BtTorrent const &torrentRef, QByteArray const &peer_id,
        QHostAddress const &ip, quint16 port)
    : BtPeer(torrentRef, peer_id, ip, port)
{

}

BtLocalPeer::BtLocalPeer(BtTorrent const &torrentRef) :
    BtPeer(torrentRef), torrent(torrentRef)
{
    torrent = torrentRef;
    setTorrentRef(torrent);
}

BtLocalPeer::BtLocalPeer(BtTorrent const &torrentRef, QByteArray const &peer_id,
        QHostAddress const &ip, quint16 port)
    : BtPeer(torrentRef, peer_id, ip, port), torrent(torrentRef)
{
    setTorrentRef(torrent);
}

/* Convert an integer to four big-endian bytes */
static QByteArray bigEndianFourBytesInteger(int integer)
{
    union {
        int i;
        quint8 s[4];
    } converter;

    converter.i = integer;
    QByteArray ret;
    ret.append(converter.s[3])
        .append(converter.s[2])
        .append(converter.s[1])
        .append(converter.s[0]);
    return ret;
}

QByteArray BtLocalPeer::handshake() const
{
    QByteArray ret("\x13");
    qDebug() << int('\x13');
    ret.append("BitTorrent protocol")
        .append("\x0\x0\x0\x0\x0\x0\x0\x0")
        .append(torrentRef.infoHash())
        .append(peer_id);
    Q_ASSERT(ret.size() == 68);
    return ret;
}

QByteArray BtLocalPeer::keepAlive() const
{
    return bigEndianFourBytesInteger(0);
}

QByteArray BtLocalPeer::choke() const
{
    return bigEndianFourBytesInteger(1).append('0');
}

QByteArray BtLocalPeer::unchoke() const
{
    return bigEndianFourBytesInteger(1).append('1');
}

QByteArray BtLocalPeer::interested() const
{
    return bigEndianFourBytesInteger(1).append('2');
}

QByteArray BtLocalPeer::notInterested() const
{
    return bigEndianFourBytesInteger(1).append('3');
}

QByteArray BtLocalPeer::have(int index) const
{
    return bigEndianFourBytesInteger(5).append('4')
        .append(bigEndianFourBytesInteger(index));
}

/* pend 0 to the spare bits */
static QByteArray bitsToBytes(QBitArray bits)
{
    QByteArray bytes;
    bytes.resize(bits.size() / 8 + 1);
    bytes.fill(0);
    for(auto b = 0; b < bits.size(); ++ b)
        bytes[b / 8] = bytes.at(b / 8) | ((bits[b]?1:0) << (b % 8));
    return bytes;
}

QByteArray BtLocalPeer::bitfield() const
{
    return bigEndianFourBytesInteger(1 + pieces.size()).append('5')
        .append(bitsToBytes(pieces));
}

QByteArray BtLocalPeer::request(int index, qint64 begin, qint64 length) const
{
    return bigEndianFourBytesInteger(13).append('6')
        .append(bigEndianFourBytesInteger(index))
        .append(bigEndianFourBytesInteger(begin))
        .append(bigEndianFourBytesInteger(length));
}

QByteArray BtLocalPeer::piece(int index, qint64 begin, qint64 length,
        QByteArray const& pieceData) const
{
    return bigEndianFourBytesInteger(9 + length).append('7')
        .append(bigEndianFourBytesInteger(index))
        .append(bigEndianFourBytesInteger(begin))
        .append(pieceData.mid(begin, length));
}

QByteArray BtLocalPeer::cancel(int index, qint64 begin, qint64 length) const
{
    return bigEndianFourBytesInteger(13).append('8')
        .append(bigEndianFourBytesInteger(index))
        .append(bigEndianFourBytesInteger(begin))
        .append(bigEndianFourBytesInteger(length));
}

QByteArray BtLocalPeer::port(quint16 listenPort) const
{
    return bigEndianFourBytesInteger(3).append('9')
        .append(listenPort >> 8).append(listenPort & 0xff);
}

bool BtLocalPeer::send(BtRemotePeer const &r, QByteArray const &message)
{
    return false;
}

void BtLocalPeer::process(QByteArray const &message)
{

}

void BtLocalPeer::received(QByteArray const &data)
{

}


