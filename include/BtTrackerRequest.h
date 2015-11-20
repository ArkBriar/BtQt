#pragma once

#ifndef __BTTRACKER_H__
#define __BTTRACKER_H__

/* This is an implementation of BitTorrent tracker request data */

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

            /* I think it's not necessary to expose all data to access. */
            QByteArray getInfoHash() const;
            QByteArray getPeerId() const;
            QHostAddress getIp() const;
            quint16 getPort() const;

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

#endif // __BTTRACKER_H__
