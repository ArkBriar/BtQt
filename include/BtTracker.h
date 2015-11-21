#pragma once

#ifndef __BTTRACKER_H__
#define __BTTRACKER_H__

/* This is an implementation of BitTorrent tracker request and response */

/* According to BPE [http://www.bittorrent.org/beps/bep_0003.html]
 * Tracker GET requests have the following keys:
 *
 * - info_hash: The 20 byte sha1 hash of the bencoded form of the info value
 *   from the metainfo file. Note that this is a substring of the metainfo
 *   file. The info-hash must be the hash of the encoded form as found in the
 *   .torrent file, regardless of it being invalid. This value will almost
 *   certainly have to be escaped.
 *
 * - peer_id: A string of length 20 which this downloader uses as its id. Each
 *   downloader generates its own id at random at the start of a new download.
 *   This value will also almost certainly have to be escaped.
 *
 * - ip: An optional parameter giving the IP (or dns name) which this peer is
 *   at. Generally used for the origin if it's on the same machine as the tracker.
 *
 * - port: The port number this peer is listening on. Common behavior is for a
 *   downloader to try to listen on port 6881 and if that port is taken try
 *   6882, then 6883, etc. and give up after 6889.
 *
 * - uploaded: The total amount uploaded so far, encoded in base ten ascii.
 *
 * - downloaded: The total amount downloaded so far, encoded in base ten ascii.
 *
 * - left: The number of bytes this peer still has to download, encoded in
 *   base ten ascii. Note that this can't be computed from downloaded and the
 *   file length since it might be a resume, and there's a chance that some of
 *   the downloaded data failed an integrity check and had to be re-downloaded.
 *
 * - event: This is an optional key which maps to started, completed, or
 *   stopped (or empty, which is the same as not being present). If not
 *   present, this is one of the announcements done at regular intervals. An a
 *   nnouncement using started is sent when a download first begins, and one
 *   using completed is sent when the download is complete. No completed is
 *   sent if the file was complete when started. Downloaders send an announcem
 *   -ent using stopped when they cease downloading.
 * */

/* Some optional keys not in bep, but may be used by some trackers.
 *
 * - compact: Setting this to 1 indicates that the client accepts a compact
 *   response. The peers list is replaced by a peers string with 6 bytes per
 *   peer. The first four bytes are the host (in network byte order), the last
 *   two bytes are the port (again in network byte order). It should be noted
 *   that some trackers only support compact responses (for saving bandwidth)
 *   and either refuse requests without "compact=1" or simply send a compact
 *   response unless the request contains "compact=0" (in which case they will
 *   refuse the request.)
 *
 * - no_peer_id: Indicates that the tracker can omit peer id field in peers
 *   dictionary. This option is ignored if compact is enabled.
 *
 * - numwant: Optional. Number of peers that the client would like to receive
 *   from the tracker. This value is permitted to be zero. If omitted, typically
 *   defaults to 50 peers.
 *
 * - key: Optional. An additional client identification mechanism that is not
 *   shared with any peers. It is intended to allow a client to prove their
 *   identity when their ip address changed.
 *
 * - trackerid: Optional. If a previous announce contained a tracker id, it
 *   should be set here.
 *
 * no_peer_id, compact, numwant is under consideration.
 */

#include <QByteArray>
#include <QString>
#include <QFile>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QUrl>

namespace BtQt {
    /* Download event */
    enum class BtTrackerDownloadEvent {
        /* empty is a default state */
        empty = 0,
        started,
        completed,
        stopped
    };

    /* Provide a function to calculate the info hash of a torrent file
     * This function will set info_hash if succeed,
     * and it will throw -1 when error occurs
     * */
    void torrentInfoHash(QFile &torrentFile, QByteArray &);

    class BtTrackerRequest {
        private:
            /* Tracker data */
            /* The variables are named as the protocol said.
             * DO NOT HATE UNDERLINES! */
            QByteArray info_hash;
            QByteArray peer_id;
            QHostAddress ip;
            quint16 port;
            quint64 uploaded;
            quint64 downloaded;
            quint64 left;
            BtTrackerDownloadEvent event;

            /* Optional */
            bool compact;
            bool no_peer_id;
            int numwant;


            /* This request will be built when first toRequestData is called!
             * Every time new data setted, this request will be cleared.
             * Next time toRequest called, it will be generated automatically.
             * */
            QByteArray requestData;
            bool requestDataGenerated;
        public:
            BtTrackerRequest();
            /* ip is optional, so it's the default value of QHostAddress when not
             * presented;
             * event has a option 'empty' */
            BtTrackerRequest(QByteArray const &, QByteArray const &, quint16, quint64, quint64, quint64, QHostAddress const & = QHostAddress(), BtTrackerDownloadEvent = BtTrackerDownloadEvent::empty);
            /* Methods */
            void setInfoHash(QByteArray const &);
            void setPeerId(QByteArray const &);
            void setIp(QHostAddress const &);
            void setPort(quint16 port);
            void setUploaded(quint64);
            void setDownloaded(quint64);
            void setLeft(quint64);
            void setEvent(BtTrackerDownloadEvent);

            void setCompact(bool);
            void setNoPeerId(bool);
            void setNumwant(int);

            /* I think it's not necessary to expose all data to access. */
            QByteArray getInfoHash() const;
            QByteArray getPeerId() const;
            QHostAddress getIp() const;
            quint16 getPort() const;
            bool isCompact() const;
            bool isNoPeerId() const;
            int getNumwant() const;

#ifndef QT_NO_DEBUG
            /* display what is in this request */
            void display() const;
#endif

            /* Get request consists of info_hash, peer_id, ip, port, uploaded,
             * downloaded, left and event */
            const QByteArray& toRequestData() const;
    };

    /* Provide a function to send request to the tracker server
     * It will throw exceptions when error following errors occured:
     * - Socket connect failed
     * - Socket read failed
     * It will warn when get an empty reply
     * */
    QByteArray sendTrackerRequest(BtTrackerRequest const &, QUrl trackerUrl);

}

/* There some facts of tracker response:
 * The tracker responds with "text/plain" document consiting of bencoded
 * dictionary with the following keys:
 *
 * - failure reason: If present, then no other keys may be present. The
 *   value is human-readable error message as to why the request failed (string)
 *
 * - warning message: (new, optional) Similar to failure reason, but the
 *   response still gets processed normally. The warning message is shown
 *   just like an error.
 *
 * - interval: interval in seconds that the client should wait between
 *   sending regular requests to the tracker
 *
 * - min interval: (optional) Minimum announce interval. If present clients
 *   must not reannouce more frequently than this.
 *
 * - tracker id: A string that the client should send back on its next
 *   announcements. If absent and a previous announce sent a tracker id, do
 *   not discard the old value; keep using it.
 *
 * - complete: number of peeers with the entire file, i.e. seeders (integer)
 *
 * - incomplete: number of non-seeder peers, aka "leechers" (integer)
 *
 * - peers: (dictionary model) The value is a list of dictionaries, each
 *   with the following keys:
 *   - peer id: peer's self-selected ID, as described above for the tracker
 *   request (string)
 *   - ip: peer's IP address either IPv6(hexed) or IPv4 (dotted quad) or
 *   DNS name (string)
 *   - port: peer's port number (integer)
 *
 * - peers: (binary model) Instead of using the dictionary model described
 *   above, the peers value may be a string consisting of multiples of 6
 *   bytes. First 4 bytes are the IP address and last 2 bytes are the port
 *   number. All in network (big endian) notation.
 *
 * Implementer's Note: Even 30 peers is plenty, the official client version
 * 3 in fact only actively forms new connections if it has less than 30
 * peers and will refuse connections if it has 55. This value is important
 * to performance. When a new piece has completed download, HAVE messages
 * will need to be sent to most active peers. As a result the cost of
 * broadcast traffic grows in direct proportion to the number of peers.
 * Above 25, new peers are highly unlikely to increase download speed. UI
 * designers are strongly advised to make this obscure and hard to change
 * as it is very rare to be useful to do so.
 *
 * Above are copied from [https://wiki.theory.org/BitTorrentSpecification]
 * */

namespace BtQt {

    /* Provide a function to deal with the response received from tracker server
     * It will throw exceptions when error following errors occured
     * */
    QMap<QString, QVariant> parseTrackerResponse(QByteArray const &);

    class BtTrackerResponse {
        private:
            /* Response data.
             * */
            int Interval;
            QByteArray TrackerId;
            int Complete;
            int InComplete;
            QList<QMap<QString, QVariant>> Peers;
            /* Optional */
            int MinInterval;
            QString failureReason;
            QString warningMessage;

        public:
            BtTrackerResponse(QMap<QString, QVariant> const &);

            /* Methods */
            /* All functions will return empty value(int: -1, bool: false)
             * when there's none.
             * */
            int interval() const;
            QByteArray trackerId() const;
            int complete() const;
            int incomplete() const;
            auto peers()->decltype(Peers) const;
            int minInterval() const;

            /* Show if there's error.
             * If true, the string will write to the input string.
             * */
            bool failed(QString &reason) const;
            bool warned(QString &warning) const;

#ifndef QT_NO_DEBUG
            /* display what is in this request */
            void display() const;
#endif
    };
}

/* Tracker 'scrape' Convention
 * By convention most trackers support another form of request, which queries
 * the state of a given torrent (or all torrents) that the tracke is managing.
 * This is referred to as the "scrape page" because it automates the otherwise
 * tedious process of "screen scraping" the tracker's stats page.
 *
 * The scrape URL is also a HTTP GET method, similar to the one described above.
 * However the base URL is different. To derive the scrape URL use the following
 * steps: Begin with the announce URL. Find the last '/' in it. If the text
 * immediately following that '/' isn't 'announce' it will be taken as a sign
 * that that tracker doesn't support the scrape convention. If it does,
 * substitute 'scrape' for 'announce' to find the scrape page.
 * Examples: (announce URL -> scrape URL)
 * +--------------------------------------------------------------------------------+
 * +    http://example.com/announce          -> http://example.com/scrape           +
 * +    http://example.com/x/announce        -> http://example.com/x/scrape         +
 * +    http://example.com/announce.php      -> http://example.com/scrape.php       +
 * +    http://example.com/a                 -> (scrape not supported)              +
 * +    http://example.com/announce?x2%0644  -> http://example.com/scrape?x2%0644   +
 * +    http://example.com/announce?x=2/4    -> (scrape not supported)              +
 * +    http://example.com/x%064announce     -> (scrape not supported)              +
 * +--------------------------------------------------------------------------------+
 *
 * The scrape URL may be supplemented by the optional parameter info_hash, a
 * 20-byte value as described above. This restricts the tracker's report to that
 * particular torrent. Otherwise stats for all torrents that the tracker is
 * managing are returned. Software authors are strongly encouraged to use the
 * info_hash parameter when at all possible, to reduce the load and bandwidth
 * of the tracker.

 * You may also specify multiple info_hash parameters to trackers that support
 * it. While this isn't part of the official specifications it has become
 * somewhat a defacto standard - for example:
 * http://example.com/scrape.php?info_hash=aaaaaaaaa&info_hash=bbbbbb
 *
 * It's not under consideration.
 * */

/* The response of scrape's HTTP GET method is a "text/plain" or sometimes
 * gzip compressed document consisting of a bencoded dictionary, containing
 * the following keys:
 * - files
 *   - complete
 *   - downloaded
 *   - incomlete
 *   - name: (optional)
 * Please go to [https://wiki.theory.org/BitTorrentSpecification] for more
 * details.
 * */

#endif // __BTTRACKER_H__
