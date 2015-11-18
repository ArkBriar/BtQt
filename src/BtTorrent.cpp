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
        qDebug() << torrentPieces.size();
        qDebug() << piecesHashList.size();
        tmpInfo["pieces"].setValue(piecesHashList);
        qDebug() << tmpInfo["pieces"].toByteArray().size();

        /* Temperarily change pieces to hashlist */
        QVariant &tInfo = torrentObject["info"];
        tInfo.setValue(tmpInfo);

        qDebug() << torrentObject["info"].toMap().value("pieces").toByteArray().size();

        BtEncodeBencodeMap(torrentObject, encoded);
        qDebug() << encoded.size();
        /* Change back */
        tInfo.setValue(torrentInfo);
    } catch (std::exception e) {
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
    } catch (std::exception e) {
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
    return torrentObject.value("annouce").toByteArray();
}

QString BtTorrent::name() const
{

    return torrentInfo.value("name").toByteArray();
}

int BtTorrent::pieceLength() const
{
    return torrentInfo.value("piece length").toByteArray().toInt();
}

QList<QVariant> BtTorrent::pieces() const
{
    return torrentPieces;
}

int BtTorrent::length() const
{
    if(torrentInfo.contains("files"))
        return -1;
    return torrentInfo.value("length").toByteArray().toInt();
}

QList<QVariant> BtTorrent::files() const
{
    if(!torrentInfo.contains("files"))
        return QList<QVariant>();
    return torrentInfo.value("files").toList();
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
