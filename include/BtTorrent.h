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

#include "BtDebug.h"
#include "BtBencode.h"

namespace BtQt {

/* Torrent file structure. All strings must be UTF-8 encoded.
 * {
 * "announce": the URL of the tracker
 *
 * "info": this maps to a dictionary whose keys are dependent on whether one or
 * more files are being shared
 *   {
 *     "name": suggested filename where the file is to be saved (if one file)/su
 *     ggested directory name where the files are to be saved (if multiple files)
 *
 *     "piece length": number of bytes per piece. This is commonly 28 KiB = 256
 *     KiB = 262,144 B.
 *
 *     "pieces": a hash list, i.e., a concatenation of each piece's SHA-1 hash.
 *     As SHA-1 returns a 160-bit hash, pieces will be a string whose length is
 *     a multiple of 160-bits. If the torrent contains multiple files, the pieces
 *     are formed by concatenating the files in the order they appear in the files
 *     dictionary (i.e. all pieces in the torrent are the full piece length
 *     except for the last piece, which may be shorter).
 *
 *     "length": size of the file in bytes (only when one file is being shared)
 *
 *     "files": a list of dictionaries each corresponding to a file (only when
 *     multiple files are being shared). Each dictionary has the following keys:
 *       {
 *         "path": a list of strings corresponding to subdirectory names, the last
 *         of which is the actual file name
 *         "length": size of the file in bytes.
 *         "md5sum": (optional) a 32-character hexadecimal string corresponding
 *         to the MD5 sum of the file. This is not used by BitTorrent at all,
 *         but it is included by some programs for greater compatibility.
 *       }
 *   }
 * }
 * */

/* BitTorrent Enhancement Proposals, known as BEP
 * Followings are copied from [http://www.bittorrent.org/beps/bep_0000.html]
 * Basic metainfo are the same as above.
 * Folloings are some metainfo extensions:
 *
 * [http://www.bittorrent.org/beps/bep_0012.html]MultiTrackers Metadata Extension
 * - announce-list: a list of trackers,
 *   example is ['announce-list']=[[tracker1], [backup1], [backup2]]. On each announce.
 *   First try tracker1, then if that canot be reached, try backup1 and backup2 respectively.
 *
 * [http://www.bittorrent.org/beps/bep_0027.html]Private Torrents
 * - private: users denote a torrent as private by including the key-value pair
 *   "private:1" in "info" dict of the torrent's metainfo file. When a BitTorrent
 *   client obtains a private torrent metainfo file, it MUST ONLY announce itself
 *   to the private tracker, and MUST ONLY initiate connections to peers returned
 *   from the private tracker.
 *
 * [http://www.bittorrent.org/beps/bep_0017.html]HTTP Seeding
 * - httpseeds: in the main area of the metadata file, will be a new key, "httpseeds".
 *   This key will refer to a list of URLs, and will contain a list of web
 *   addresses where torrent data can be retrieved. Examples are:
 *    ['httpseeds'] = [ 'http://www.site1.com/source1.php', 'http://www.site2.com/source2.php' ]
 *
 * [http://www.bittorrent.org/beps/bep_0005.html]DHT Protocol
 * - nodes: nodes = [["<host>", <port>], ["<host>", <port>], ...]
 *   A trackerless torrent dicktionary does not have an "announce" key. Instead,
 *   a trackerless torrent has a "nodes" key. For more speicifications please go
 *   to the link above.
 *
 * We plan to support 'announce-list', 'private', 'httpseeds' and DHT.
 * Merkle trees is not considered.
 */

/* All above are necessary info. There are some optional kyes:
 *
 * - creation date: the creation time of the torrent, in standard UNIX epoch
 *   format (integer, seconds since 1-Jan-1970 00:00:00 UTC)
 *
 * - comment: free-form textual comments of the author (string)
 *
 * - created by: name and version of the program use to create the .torrent (string)
 *
 * - encoding: (optional) the string encoding format used to generate the pieces
 *   part of the info dictionary in the .torrent metafile (string)
 *
 * I have no idea what values encoding has, so it's not under consideration.
 * This program will sign it's own torrents with 'creation date', 'created by: BtQt [Version]', 'comment'(optional)
 * */

    class BtTorrent {
        private:
            /* Data
             * How to store the torrent data?
             * In Json or some Qt structure?
             * */
            //QJsonObject torrentJsonObject;
            QMap<QString, QVariant> torrentObject;

            /* Index, built when decode or setvalue is done */
            QMap<QString, QVariant> torrentInfo;
            QList<QVariant> torrentPieces;

            bool isParsed;

            /* If this is a valid torrent object, return true;
             * else return false
             * */
            bool isValid();

        public:
            /* Construct with isParsed be false */
            BtTorrent() : isParsed(false) {}

            /* Methods */
#ifndef QT_NO_DEBUG
            /* Provide a method to print the data
             * display()
             * */
            void display() const;
#endif // QT_NO_DEBUG

            /* Get specific data from torrent object */
            QString announce() const;
            QString name() const;
            qint64 pieceLength() const;
            QList<QByteArray> pieces() const;
            /* When it's a single-file torrent, return empty QList
             * else return with files
             * */
            QList<QMap<QString, QVariant>> files() const;
            /* When it's a multiple-files torrent, return -1
             * else return length
             * */
            qint64 length() const;

            /* Optional information
             * Return empty structure when not found in meta object,
             * and return -1 when type is int, false when bool
             * */
            QList<QString> announceList() const;
            bool isPrivate() const;
            QList<QString> httpseeds() const;
            /* For DHT */
            QList<QPair<QString, int>> nodes() const;

            QString creationDate() const;
            QString comment() const;
            QString createdBy() const;
#ifndef BT_NO_DEPRECATED_FUNCTION
            QString encoding() const;
#endif // BT_NO_DEPRECATED_FUNCTION

            /* Provide some funtions to set part of those options */
            void setCreationDate(QString const &);
            void setComment(QString const &);
            void setCreateBy(QString const &);
#ifndef BT_NO_DEPRECATED_FUNCTION
            /* Encodint will never take effect in BtQt */
            void setEncoding(QString const &);
#endif // BT_NO_DEPRECATED_FUNCTION

            /* Set value
             * This function would always destory data in the argument
             * If input is not valid, return fasle
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
             * decodeTorrentFile(QFile &)
             * */
            bool decodeTorrentFile(QFile &torrentFile);

            /* Provide a method to parse itself to a torent file
             * encodeTorrentFile(QFile &)
             * */
            bool encodeTorrentFile(QFile &torrentFile);
    };
}

#endif // __BTTORRENT_H__
