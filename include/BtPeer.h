#pragma once

/* This is an implementation of bittorent peer protocol */

/* Peer wire protocal [https://wiki.theory.org/BitTorrentSpecification]
 *
 * #Data Types
 * Unless specified otherwise, all integers in the peer wire protocol are
 * encoded as four big-endian values. This includes the length prefix on all
 * messages that come after the handshake.
 *
 * #Message flow
 * The peer wire protocol consists of an initial handshake. After that, peers
 * communicate via an exchange of length-prefixed messages. The length-prefix is
 * an integer as described above.
 *
 * #Handshake
 * The handshake is a required message and must be the first message transmitted
 * by the client. It is (49+len(pstr)) bytes long.
 *
 * handshake: <pstrlen><pstr><reserved><info_hash><peer_id>
 * - pstrlen: string length of <pstr>, as a single raw byte
 * - pstr: string indetifier of the protocol
 * - reserved: eight reserved bytes. All current implementations use all zeroes.
 * - info_hash: 20-byte SHA1 hash of the info key in the metainfo file. This is
 *   the same info_hash that is transmitted in tracker requests.
 * - peer_id: 20-byte string used as a unique ID for the client. This is usually
 *   the same peer_id that is transmitted in tracker requests (but not always
 *   e.g.  an anonymity option in Azureus)
 *
 * In version 1.0 of the BitTorrent protocol, pstrlen = 19, and pstr =
 * "BitTorrent protocol".
 *
 * The initiator of a connection is expected to transmit their handshake
 * immediately. The recipient may wait for the initiator's handshake, if it is
 * capable of serving multiple torrents simultaneousely (torrents are uniquely
 * identified by their infohash). However, the recipient must respond as soon as
 * it sees the info_hash part of the handshake (the peer id will presumably be
 * sent after the recipient seeds its own handshake). The tracker's NAT-checking
 * feature does not send the peer_id field of the handshake.
 *
 * If a client receives a handshake with an info_hash that it is not currently
 * serving, then the client must drop the connection.
 *
 * If the initiator of the connection receives a handshake in which the peer_id
 * does not match the expected peer id, then the initiator is expected to drop
 * the connection. Note that then initiator presumably received the peer
 * infomation form the tracker, which includes the peer_id that was registered
 * by the peer. The peer_id from the tracker and in the handshake are expected
 * to match.
 *
 * #Messages
 * All of the remaining messages in the protocol take the form of <lengthprefix>
 * <message ID><payload>. The length prefix is a four byte big-endian value. The
 * message Id is a single decimal byte. The payload is message dependent.
 *
 * - keep-alive: <len=0000>
 *   The keep-alive message is a message with zero bytes, specified with the
 *   length prefix set to zero. There is no message ID and no payload. Peers may
 *   close a connection if they receive no messages (keep-alive or any other
 *   message) for a certain period of time, so a keep-alive message must be sent
 *   to maintain the connection alive if no command have sent for a given amount
 *   of time. This amount of time is generally two minutes.
 *
 * - choke: <len=0001><id=0>
 *   The choke message is fixed-length and has no payload.
 *
 * - unchoke: <len=0001><id=1>
 *   The unchoke message is fixed-length and has no payload.
 *
 * - interested: <len=0001><id=2>
 *   The interested message is fixed-length and has no payload.
 *
 * - not interested: <len=0001><id=3>
 *   The not interested message is fixed-length and has no payload.
 *
 * - have: <len=0005><id=4><piece index>
 *   The have message is fixed length. The payload is the zero-based index of a
 *   piece that has just been successfully downloaded and verified via the hash.
 *
 *   Implementer's Note: That is the strict definition, inreality some games may
 *   be played. In particular because peers are extremely unlikely to download
 *   pieces that they already have, a peer may choose not to advertise having a
 *   piece to a peer that already has that piece. At a minimum "HAVE suppression"
 *   will result in a 50% reduction in the number of HAVE messages, this
 *   translates to around a 25%-35% reduction in protocol overhead. At the same
 *   time, it may be worthwhile to send A HAVE message to a peer that has that
 *   piece already since it will be useful in determining which piece is rare.
 *
 *   A malicious peer might also choose to advertise having pieces that it *   knows the peer will never download. Due to this attempting to model peers
 *   using this information is a **BAD IDEA**.
 *
 * - bitfield: <len=0001+X><id=5><bitfield>
 *   The bitfield messge may only be sent immediately after the handshaking
 *   sequence is completed, and before any other messages are sent. It is
 *   optional, and need not be sent if a client has no pieces.
 *
 *   The bitfield message is variable length, where X is the length of the
 *   bitfield. The payload is bitfield representing the pieces that have been
 *   successfully downloaded. The high bit in the first byte corresponds to
 *   piece index 0. Bits that are cleared indicated a missing piece, and set
 *   bits indicate a valid and available piece. Spare bits at the end are set to
 *   zero.
 *
 *   Some clients send bitfield with missing pieces even if it has all data.
 *   Then it sends rest of pieces as have messages. They are saying this helps
 *   against ISP filtering of BitTorrent protocol. It is called lazy bitfield.
 *
 *   A bitfield of the wrong length is considered an error. Clients should drop
 *   the connection if they receive bitfields that are not of the correct size,
 *   or if the bitfield has any of the spare bits set.
 *
 * - request: <len=0013><id=6><index><begin><length>
 *   The request message is fixed legnth, and is used to request a block.
 *   The payload contains the following information:
 *     - index: integer specifying the zero-based piece index
 *     - begin: integer specifying the zero-based byte offset within the piece
 *     - length: integer specifying the requested length
 *   There is a discuss of the maximum size of request is suitable due to a
 *   historical problem. For more information please go to [https://wiki.theory.org/BitTorrentSpecification]
 *
 * - piece: <len=0009+X><id=7><index><begin><block>
 *   The piece message is variable length, where X is the length of the block.
 *   The payload contains the following information:
 *     - index: integer specifying the zero-based piece index
 *     - begin: integer specifying the zero-based byte offset within the piece
 *     - block: block of data, which is a subset of the piece speicified by index
 * - cancel: <len=0013><id=8><index><begin><length>
 *   The cancel message is fixed length, and is used to cancel block requests.
 *   The payload is identical to that of the "request" message. It is typically
 *   used during "End Game" (one of the algorithms)
 *
 * - port: <len=0003><id=9><listen-port>
 *   The port message is sent by newer versions of the Mainline that implements
 *   a DHT tracker. The listen port is the port this peer's DHT node is
 *   listening on. This peer should be inserted in the local routing table
 *   (if DHT tracker is supported).
 */

 /* There are some offcial extensions to the protocol
  *
  * - Fast Peers Extensions
  * - Distributed Hash Table
  * - Connection Obfuscation
  *
  * And some unoffcial extensions
  *
  * - Extension protocol
  *
  */

#ifndef __BTPEER_H__
#define __BTPEER_H__

#include <QByteArray>
#include <QBitArray>
#include <QHostAddress>
#include <BtTorrent.h>
#include <QTcpSocket>
#include <QUdpSocket>

NAMESPACE_BEGIN(BtQt)
/* Provide a function to generate peer id, never fails
 * There are mainly two conventions how to encode client and client version
 * information into the peer_id, Azureus-style and Shadow's-style.
 * Azureus-style uses the following encoding: '-', two characters for client
 * id, four ascii digits for version number, '-', followed by random numbers.
 * We use Azureus-style id and use 'QB' for BtQt client id.
 * Don't forget to do qsrand() before call this function.
 * */
QByteArray generatePeerId();

/* BtPeer is an object that maintain information of peers, both local and
 * remote.
 *
 * BtPeer will provide a set of method to complete communicate with other
 * peers
 * */
class BtPeer {
protected:
    /* Data to store */
    QByteArray peer_id;
    QHostAddress peerIp;
    quint16 peerPort;

    bool AmChoking;
    bool AmInterested;
    bool PeerChoking;
    bool PeerInterested;

    /* Specify what torrent this peer has */
    const BtTorrent& torrentRef;
    /* Specify pieces this peer has */
    QBitArray pieces;

public:
    BtPeer(BtTorrent const &);
    BtPeer(BtTorrent const &, QByteArray const &,
            QHostAddress const &, quint16 = 6881);


    /* Set peer data */
    void setPeerId(QByteArray const &);
    void setIp(QHostAddress const &);
    void setPort(quint16);
    void setTorrentRef(BtTorrent const &);

    void pieceOk(int);

    /* Get peer info */
    QByteArray getPeerId() const;
    QHostAddress getIp() const;
    quint16 getPort() const;
    const BtTorrent& getTorrentRef() const;
    bool amChoking() const;
    bool amInterested() const;
    bool peerChoking() const;
    bool peerInterested() const;
    QBitArray getPieceBitArray() const;
    bool havePiece(int) const;
};

class BtRemotePeer : public BtPeer {
public:
    BtRemotePeer(BtTorrent const &);
    BtRemotePeer(BtTorrent const &, QByteArray const &,
            QHostAddress const &, quint16 = 6881);
};

/* Local Peer is declared to be QObject so that we can use
 * signal-slot mechanism */
class BtLocalPeer : public QObject, public BtPeer {
    Q_OBJECT

private:
    /* This is copy of torrent this peer serves */
    BtTorrent torrent;

    /* Socket for communication */
    QTcpSocket tcpSocket;
    QUdpSocket udpSocket;

public:
    BtLocalPeer(BtTorrent const &);
    BtLocalPeer(BtTorrent const &, QByteArray const &,
            QHostAddress const &, quint16 = 6881);
    /* These are all over TCP */
    QByteArray handshake() const;
    /* Implementation of messages */
    QByteArray keepAlive() const;
    QByteArray choke() const;
    QByteArray unchoke() const;
    QByteArray interested() const;
    QByteArray notInterested() const;
    QByteArray have(int index) const;
    QByteArray bitfield() const;
    QByteArray request(int index, qint64 begin, qint64 length) const;
    /* pieceData is correspongding to piece of index */
    QByteArray piece(int index, qint64 begin, qint64 length,
            QByteArray const& pieceData) const;
    QByteArray cancel(int index, qint64 begin, qint64 length) const;
    QByteArray port(quint16 listenPort) const;

    /* Process messages */
    /* Send messages, return true if sent */
    bool send(BtRemotePeer const &, QByteArray const &);

    /* Process message and decide what to do next */
    void process(QByteArray const &);
public slots:
    void received(QByteArray const &);
};
NAMESPACE_END(BtQt)

#endif // __BTPEER_H__
