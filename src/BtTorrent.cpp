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
    qStdOut(tabs) << "]" << endl;
}

void BtTorrent::display()
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

bool BtTorrent::decodeTorrentFile(QFile &torrentFile)
{
    torrentObject.clear();
    if(!torrentFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file " << torrentFile.fileName() << " in read-only mode.";
        throw -1;
    }

    QByteArray torrentData = torrentFile.readAll();
    torrentFile.close();

    try {
        BtQt::BtDecodeBencodeDictionary(torrentData, torrentObject);
    } catch (std::exception e) {
        qDebug() << "Cannot decode torrent file " << torrentFile.fileName() << "!";
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

    isParsed = true;
    return true;
}

