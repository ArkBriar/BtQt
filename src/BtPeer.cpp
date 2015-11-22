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

BtPeer::BtPeer(BtTorrent const &torrent, bool isRemote)
    : AmChoking(false), AmInterested(false), PeerChoking(false),
    PeerInterested(false), torrent(torrent), IsRemote(isRemote)
{

}

BtPeer::BtPeer(BtTorrent const &torrent, QByteArray const &peer_id,
        QHostAddress const &ip, quint16 port, bool isRemote)
    : peer_id(peer_id), peerIp(ip), peerPort(port), AmChoking(false),
    AmInterested(false), PeerChoking(false), PeerInterested(false),
    torrent(torrent), IsRemote(isRemote)
{

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

void BtPeer::setTorrent(BtTorrent const &torrent)
{
    const_cast<BtTorrent &>(this->torrent) = torrent;
}

void BtPeer::setIsRemote(bool isRemote)
{
    IsRemote = isRemote;
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

const BtTorrent& BtPeer::getTorrent() const
{
    return torrent;
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

bool BtPeer::isRemote() const
{
    return IsRemote;
}

bool BtPeer::havePiece(int idx) const
{
    return pieces.testBit(idx);
}
