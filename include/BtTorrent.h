#pragma once

#ifndef __BTTORRENT_H__
#define __BTTORRENT_H__

#include <QFile>
#include <QMap>
#include <QVariant>
/* Use QJson to store torrent data */
/*
 *#include <QJsonObject>
 *#include <QJsonDocument>
 *#include <QJsonParseError>
 *#include <QJsonValue>
 *#include <QJsonValueRef>
 */

/* Macros for debug */
#ifndef QT_NO_DEBUG
#ifdef __GNUC__
#define showDebug() \
    qDebug() << "[DEBUG] " << __PRETTY_FUNCTION__ << __LINE__
#else
#define showDebug() \
    qDebug() << "[DEBUG] " << __FUNCTION__ << __LINE__
#endif

#else
#define showDebug()
#endif

#include "BtBencode.h"

namespace BtQt {

/* Torrent file structure. All strings must be UTF-8 encoded.
 * {
 * "announce": the URL of the tracker
 * "info": this maps to a dictionary whose keys are dependent on whether one or
 * more files are being shared
 *   {
 *     "name": suggested filename where the file is to be saved (if one file)/su
 *     ggested directory name where the files are to be saved (if multiple files)
 *     "piece length": number of bytes per piece. This is commonly 28 KiB = 256
 *     KiB = 262,144 B.
 *     "pieces": a hash list, i.e., a concatenation of each piece's SHA-1 hash.
 *     As SHA-1 returns a 160-bit hash, pieces will be a string whose length is
 *     a multiple of 160-bits. If the torrent contains multiple files, the pieces
 *     are formed by concatenating the files in the order they appear in the f
 *     iles dictionary (i.e. all pieces in the torrent are the full piece length
 *     except
 *     for the last piece, which may be shorter).
 *     "length": size of the file in bytes (only when one file is being shared)
 *     "files": a list of dictionaries each corresponding to a file (only when
 *     multiple files are being shared). Each dictionary has the following keys:
 *       {
 *         "path": a list of strings corresponding to subdirectory names, the last
 *         of which is the actual file name
 *         "length": size of the file in bytes.
 *       }
 *   }
 * }
 * */

/* BitTorrent Enhancement Proposals, known as BEP
 * Followings are copied from [http://www.bittorrent.org/beps/bep_0000.html]
 * Metainfo are the same as above.
 * Folloings are some metainfo extensions:
 * [http://www.bittorrent.org/beps/bep_0012.html]MultiTrackers Metadata Extension
 * - announce-list: a list of trackers,
 *   example is ['announce-list']=[[tracker1], [backup1], [backup2]]. On each announce.
 *   First try tracker1, then if that canot be reached, try backup1 and backup2 respectively.
 * [http://www.bittorrent.org/beps/bep_0027.html]Private Torrents
 * - private: users denote a torrent as private by including the key-value pair
 *   "private:1" in "info" dict of the torrent's metainfo file. When a BitTorrent
 *   client obtains a private torrent metainfo file, it MUST ONLY announce itself
 *   to the private tracker, and MUST ONLY initiate connections to peers returned
 *   from the private tracker.
 * [http://www.bittorrent.org/beps/bep_0017.html]HTTP Seeding
 * - httpseeds: in the main area of the metadata file, will be a new key, "httpseeds".
 *   This key will refer to a list of URLs, and will contain a list of web
 *   addresses where torrent data can be retrieved. Examples are:
 *    ['httpseeds'] = [ 'http://www.site1.com/source1.php', 'http://www.site2.com/source2.php' ]
 * [http://www.bittorrent.org/beps/bep_0005.html]DHT Protocol
 * - nodes: nodes = [["<host>", <port>], ["<host>", <port>], ...]
 *   A trackerless torrent dicktionary does not have an "announce" key. Instead,
 *   a trackerless torrent has a "nodes" key. For more speicifications please go
 *   to the link above.
 *
 * We plan to support 'announce-list', 'private' and 'httpseeds'.
 * Merkle trees and DHT is not considered.
 */

    class BtTorrent {
        private:
            /* Data
             * How to store the torrent data?
             * In Json or some Qt structure?
             * */
            //QJsonObject torrentJsonObject;
            QMap<QString, QVariant> torrentObject;

            /* Index, lazy built */

            bool isParsed;

            /* If this is a valid torrent object, return true;
             * else return false
             * */
            bool isValid();

        public:
            /* Construct with isParsed be false */
            BtTorrent() : isParsed(false) {}

            /* Methods */
            /* Provide a method to print the data
             * display()
             * */
            void display() const;

            /* Get specific data from torrent object */
            QString announce() const;
            QString name() const;
            int pieceLength() const;
            QList<QString> pieces() const;
            /* When it's a single-file torrent, return empty QList
             * else return with files
             * */
            QList<QVariant> files() const;
            /* When it's a single-file torrent, return -1
             * else return length
             * */
            int length() const;

            /* Set value
             * This function would always destory data in the argument
             * If input is no valid, return fasle
             * else return true
             * */
            bool setValue(QMap<QString, QVariant> &);

            /* get a copy of value */
            QMap<QString, QVariant> value() const;

            /* Clear
             * Reset data
             * */
            void clear();

            /* Provide a method to parse torrent file
             * decodeTorrentFile(QFile const&)
             * will throw exceptions when error occurs
             * */
            bool decodeTorrentFile(QFile &torrentFile);

            /* Provide a method to parse itself to a torent file
             * encodeTorrentFile(QFile const&)
             * will throw exceptions when error occurs
             * */
            /* Not implemented */
            //void encodeTorrentFile(QFile const &torrentFile);
    };
}

#endif // __BTTORRENT_H__
