#pragma once

#ifndef __BTTRACKER_H__
#define __BTTRACKER_H__

/* This is an implementation of BitTorrent tracker */

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
 * - left: The number of bytes this peer still has to download, encoded in
 *   base ten ascii. Note that this can't be computed from downloaded and the
 *   file length since it might be a resume, and there's a chance that some of
 *   the downloaded data failed an integrity check and had to be re-downloaded.
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

namespace BtQt {
    /* Trackers event */
    enum class BtTrackerEvent {
        started,
        completed,
        stopped
    };

    class BtTracker {
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
            BtTrackerEvent event;

        public:
            /* Methods */
            /* Provide a function to calculate the info hash of a torrent file
             * This function will set info_hash if succeed,
             * and it will throw -1 when error occurs
             * */
            void torrentInfoHash(QFile torrentFile);

            /* Provide a function to encode itself to Bencode
             * It will throw -1 when error occurs
             * */
            void BencodeEncode(QByteArray &);
    };
}

#endif // __BTTRACKER_H__
