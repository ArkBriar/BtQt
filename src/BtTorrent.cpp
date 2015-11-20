#include "BtTorrent.h"
#include <QByteArray>
#include <QIODevice>
#include <QDebug>
#include <QTextDecoder>
#include <QTextCodec>
#include <QMetaType>
#include <QTextStream>

using namespace BtQt;

static QTextStream& qStdOut(int tabs = 0, bool useBlank = true) {
    static QTextStream ts(stdout);
    for (auto i = 0; i < tabs; i ++) {
        if(useBlank) ts << "  ";
        else ts << "\t";
    }
    return ts;
}

static void displayQMap(QMap<QString, QVariant> const &, int = 0);
static void displayQList(QList<QVariant> const &, int = 0);

static void displayQMap(QMap<QString, QVariant> const &map, int tabs)
{
    qStdOut(tabs) << "{" << endl;
    ++ tabs;
    for(auto key : map.keys()) {
        qStdOut(tabs) << key << ": ";
        QVariant value = map.value(key);

        if(value.canConvert(QMetaType::QString)) {
            qStdOut() << value.toString() << endl;
        } else if(value.canConvert(QMetaType::QByteArray)) {
            qStdOut() << value.toByteArray() << endl;
        } else if(value.canConvert(QMetaType::QVariantList)) {
            qStdOut() << endl;
            displayQList(value.toList(), tabs);
        } else if(value.canConvert(QMetaType::QVariantMap)) {
            qStdOut() << endl;
            displayQMap(value.toMap(), tabs);
        } else {
            qDebug() << "There are illegal types.";
        }
    }
    -- tabs;
    qStdOut(tabs) << "}" << endl;
}

static void displayQList(QList<QVariant> const &list, int tabs)
{
    qStdOut(tabs) << "[" << endl;
    ++ tabs;
    for(auto value : list) {
        if(value.canConvert(QMetaType::QString)) {
            qStdOut(tabs) << value.toString() << endl;
        } else if(value.canConvert(QMetaType::QByteArray)) {
            qStdOut(tabs) << value.toByteArray() << endl;
        } else if(value.canConvert(QMetaType::QVariantList)) {
            displayQList(value.toList(), tabs);
        } else if(value.canConvert(QMetaType::QVariantMap)) {
            displayQMap(value.toMap(), tabs);
        } else {
            qDebug() << "There are illegal types.";
        }
    }
    -- tabs;
    qStdOut(tabs) << "]" << endl;
}

#ifndef QT_NO_DEBUG
void BtTorrent::display() const
{
    if(!isParsed) {
        qDebug() << "BtTorrent object is empty.";
        return;
    }

    displayQMap(torrentObject);
    /*
     *qInfo() << torrentObject;
     */
}
#endif // QT_NO_DEBUG

bool BtTorrent::encodeTorrentFile(QFile &torrentFile)
{
    if(!isParsed) {
        qDebug() << "Can not encode an empty object.";
        return false;
    }

    QByteArray encoded;
    try {
        /* The pieces's value is sha-1 lists (hex encoded),
         * so we should join the lists together */
        QMap<QString, QVariant> tmpInfo = torrentInfo;
        QByteArray piecesHashList;
        for(auto i : torrentPieces) {
            auto pieceHash = QByteArray::fromHex(i.toByteArray());
            piecesHashList.append(pieceHash);
        }
        tmpInfo["pieces"].setValue(piecesHashList);

        /* Make a copy of torrentObject,
         * and change pieces to hashlist */
        QMap<QString, QVariant> tmpObject = torrentObject;
        QVariant &tInfo = tmpObject["info"];
        tInfo.setValue(tmpInfo);

        BtEncodeBencodeMap(tmpObject, encoded);
    } catch (int e) {
        qDebug() << "Can not encode this object. Error occurs!";
        return false;
    }

    if(!torrentFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Can not open file " << torrentFile.fileName() << " in write-only mode.";
        return false;
    }

    if(torrentFile.write(encoded) == -1) {
        qDebug() << "Can not write to file " << torrentFile.fileName();
        torrentFile.close();
        return false;
    }

    torrentFile.close();
    return true;
}

bool BtTorrent::decodeTorrentFile(QFile &torrentFile)
{
    torrentObject.clear();
    if(!torrentFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Can not open file " << torrentFile.fileName() << " in read-only mode.";
        return false;
    }

    QByteArray torrentData = torrentFile.readAll();
    torrentFile.close();

    try {
        BtQt::BtDecodeBencodeDictionary(torrentData, torrentObject);
    } catch (int e) {
        qDebug() << "Can not decode torrent file " << torrentFile.fileName() << "!";
        return false;
    }

    /* Parse pieces */
    QVariant& tInfo = torrentObject["info"];
    /* There is no "info" */
    if(tInfo == QVariant() && tInfo.canConvert(QMetaType::QVariantMap)) {
        qDebug() << "Torrent is broken";
        return false;
    }

    QVariant tPieces = tInfo.toMap().value("pieces");
    /* There is no "pieces" */
    if(tPieces == QVariant() && tPieces.canConvert(QMetaType::QByteArray)) {
        qDebug() << "Torrent is broken";
        return false;
    }

    QList<QVariant> hashList;

    for(auto i = 0; i < tPieces.toByteArray().size(); i += 160 / 8) {
        hashList.push_back(tPieces.toByteArray().mid(i, 160 / 8).toHex());
    }

    auto tInfoVal = tInfo.toMap();
    tInfoVal.remove("pieces");
    tInfoVal.insert("pieces", hashList);
    tInfo.setValue(tInfoVal);

    isParsed = isValid();

    /* Build index */
    if(isParsed) {
        torrentInfo = torrentObject.value("info").toMap();
        torrentPieces = torrentInfo.value("pieces").toList();
    }

    return isParsed;
}

bool BtTorrent::isValid()
{
    /* Check the standard structure */
    if(!torrentObject.contains("announce"))
        return false;
    if(!torrentObject.contains("info"))
        return false;

    auto tInfo = torrentObject.value("info");

    if(!tInfo.canConvert(QMetaType::QVariantMap))
        return false;
    auto tInfoMap = tInfo.toMap();
    if(!tInfoMap.contains("name"))
        return false;
    if(!tInfoMap.contains("piece length"))
        return false;

    /* Pieces */
    if(!tInfoMap.contains("pieces"))
        return false;
    auto tPieces = tInfoMap.value("pieces");
    if(!tPieces.canConvert(QMetaType::QVariantList))
        return false;

    /* Files
     * Files only exists when there are multiple files */
    if(tInfoMap.contains("files")) {
        /* Multiple files */
        auto tFiles = tInfoMap.value("files");
        if(!tFiles.canConvert(QMetaType::QVariantList))
            return false;
        auto tFilesList = tFiles.toList();
        for (auto i : tFilesList) {
            if(!i.canConvert(QMetaType::QVariantMap))
                return false;
            auto tFile = i.toMap();
            if(!tFile.contains("path") || !tFile.contains("length"))
                return false;
            auto tFilePath = tFile.value("path");
            if(!tFilePath.canConvert(QMetaType::QVariantList))
                return false;
        }
    } else {
        /* Single file */
        if(!tInfoMap.contains("length"))
            return false;
    }

    return true;
}

QString BtTorrent::announce() const
{
    return torrentObject.value("announce").toByteArray();
}

QString BtTorrent::name() const
{

    return torrentInfo.value("name").toByteArray();
}

qint64 BtTorrent::pieceLength() const
{
    return torrentInfo.value("piece length").toByteArray().toLongLong();
}

QList<QByteArray> BtTorrent::pieces() const
{
    QList<QByteArray> ret;
    for (auto i : torrentPieces) {
        ret.append(i.toByteArray());
    }

    return ret;
}

qint64 BtTorrent::length() const
{
    if(torrentInfo.contains("files"))
        return -1;
    return torrentInfo.value("length").toByteArray().toLongLong();
}

QList<QMap<QString, QVariant>> BtTorrent::files() const
{
    if(!torrentInfo.contains("files"))
        return QList<QMap<QString, QVariant>>();
    auto fileList =  torrentInfo.value("files").toList();

    QList<QMap<QString, QVariant>> ret;
    for (auto i : fileList) {
        ret.append(i.toMap());
    }

    return ret;
}

QMap<QString, QVariant> BtTorrent::value() const
{
    return torrentObject;
}

bool BtTorrent::setValue(QMap<QString, QVariant> &other)
{
    torrentObject.swap(other);
    isParsed = isValid();

    /* Build index */
    if(isParsed) {
        torrentInfo = torrentObject.value("info").toMap();
        torrentPieces = torrentInfo.value("pieces").toList();
    }

    return isParsed;
}

void BtTorrent::clear()
{
    isParsed = false;
    torrentObject.clear();
}

bool BtTorrent::isPrivate() const
{
    if(!torrentInfo.contains("private")) return false;
    bool ok;
    int privateVal = torrentInfo.value("private").toInt(&ok);
    if(ok && privateVal == 1) return true;

    return false;
}

QString BtTorrent::creationDate() const
{
    if(!torrentObject.contains("creation date")) return QString();

    if(torrentObject.value("creation date").canConvert(QMetaType::QString))
        return torrentObject.value("creation date").toString();

    return QString();
}

QString BtTorrent::comment() const
{
    if(!torrentObject.contains("comment")) return QString();

    if(torrentObject.value("comment").canConvert(QMetaType::QString))
        return torrentObject.value("comment").toString();
    return QString();
}

QString BtTorrent::createdBy() const
{
    if(!torrentObject.contains("created by")) return QString();

    if(torrentObject.value("created by").canConvert(QMetaType::QString))
        return torrentObject.value("created by").toString();
    return QString();
}

QList<QString> BtTorrent::announceList() const
{
    if(!torrentObject.contains("announce-list")) return QList<QString>();

    if(torrentObject.value("announce-list").canConvert(
                QMetaType::QVariantList)) {
        QList<QString> ret;
        for (auto i : torrentObject.value("announce-list").toList()) {
            /* I have to say that always check is not a good choice.
             * Shit, there are so much to check. */
            ret.append(i.toString());
        }
        return ret;
    }

    return QList<QString>();
}

QList<QString> BtTorrent::httpseeds() const
{
    if(!torrentObject.contains("httpseeds")) return QList<QString>();

    if(torrentObject.value("httpseeds").canConvert(QMetaType::QVariantList)) {
        QList<QString> ret;
        for (auto i : torrentObject.value("httpseeds").toList()) {
            ret.append(i.toString());
        }
        return ret;
    }

    return QList<QString>();
}

QList<QPair<QString, int>> BtTorrent::nodes() const
{
    if(!torrentObject.contains("nodes") ||
            !torrentObject.value("nodes").canConvert(QMetaType::QVariantList))
        return QList<QPair<QString ,int>>();

    auto nodesList = torrentObject.value("nodes").toList();
    QList<QPair<QString, int>> ret;

    for (auto i : nodesList) {
        auto host_port = i.toList();
        QString host = host_port.at(0).toString();
        int port = host_port.at(1).toInt();
        ret.append(QPair<QString, int>(host, port));
    }

    return ret;
}

#ifndef BT_NO_DEPRECATED_FUNCTION
QString BtTorrent::encoding() const
{
    if(!torrentObject.contains("encoding")) return QString();

    if(torrentObject.value("encoding").canConvert(QMetaType::QString))
        return torrentObject.value("encoding").toString();
    return QString();
}

void BtTorrent::setEncoding(QString const &encoding)
{
    torrentObject.insert("encoding", encoding);
}
#endif // BT_NO_DEPRECATED_FUNCTION

void BtTorrent::setCreationDate(QString const &date)
{
    torrentObject.insert("creation date", date);
}

void BtTorrent::setComment(QString const &comment)
{
    torrentObject.insert("comment", comment);
}

void BtTorrent::setCreateBy(QString const &tool)
{
    torrentObject.insert("created by", tool);
}
