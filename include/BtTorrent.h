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
    class BtTorrent {
        private:
            bool isParsed;
        public:
            /* Construct with isParsed be false */
            BtTorrent() : isParsed(false) {}

            /* Data
             * How to store the torrent data?
             * In Json or some Qt structure?
             * */
            //QJsonObject torrentJsonObject;
            QMap<QString, QVariant> torrentObject;

            /* Methods */
            /* Provide a method to print the data
             * display()
             * */
            void display();

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

/* Torrent file structure. All strings must be UTF-8 encoded.
 * {
 * "announce": the URL of the tracker
 * "info": this maps to a dictionary whose keys are dependent on whether one or more files are being shared
 *   {
 *     "name": suggested filename where the file is to be saved (if one file)/suggested directory name where the files are to be saved (if multiple files)
 *     "piece length": number of bytes per piece. This is commonly 28 KiB = 256 KiB = 262,144 B.
 *     "pieces": a hash list, i.e., a concatenation of each piece's SHA-1 hash. As SHA-1 returns a 160-bit hash, pieces will be a string whose length is a multiple of 160-bits. If the torrent contains multiple files, the pieces are formed by concatenating the files in the order they appear in the files dictionary (i.e. all pieces in the torrent are the full piece length except for the last piece, which may be shorter).
 *     "length": size of the file in bytes (only when one file is being shared)
 *     "files": a list of dictionaries each corresponding to a file (only when multiple files are being shared). Each dictionary has the following keys:
 *       {
 *         "path": a list of strings corresponding to subdirectory names, the last of which is the actual file name
 *         "length": size of the file in bytes.
 *       }
 *   }
 * }
 * */

#endif // __BTTORRENT_H__
