#include "BtBencode.h"
#include <QtGlobal>
#include <QDebug>

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

using namespace BtQt;

/* if fastDecode turns to true, decode functions below will skip
 * some necessary check */
static bool fastDecode = false;

/* The two functions below do not handle 'd' and 'l'
 * if error occurs or reach end, return -1 */
static inline int pickAndDecode(QByteArray const &data, int pos, QByteArray &ret) {
    showDebug();
    if(pos > (int)data.size() - 1) return -1;
    if(data[pos] != 'i' && data[pos] != 'l' && data[pos] != 'd') {
        // is string
        int colonPos = data.indexOf(':', pos);
        int len = data.mid(pos, colonPos - pos).toInt();
        try {
            BtDecodeBencodeString(data.mid(pos, colonPos - pos + len + 1), ret);
        } catch (std::exception e){
            qDebug() << "Bencode string decode error, the input is: " << data.mid(pos, colonPos - pos + len + 1);
            goto PICK_AND_DECODE_ERROR;
        }
        return colonPos + len + 1;
    } else if(data[pos] == 'i') {
        // is integer
        int ePos = data.indexOf('e', pos);
        try {
            BtDecodeBencodeInteger(data.mid(pos, ePos - pos + 1), ret);
        } catch (std::exception e) {
            qDebug() << "Bencode string decode error, the input is: " << data.mid(pos, ePos - pos + 1);
            goto PICK_AND_DECODE_ERROR;
        }
        return ePos + 1;
    }

PICK_AND_DECODE_ERROR:
    return -1;
}

void BtQt::BtDecodeBencodeInteger(QByteArray const &data, QByteArray &ret)
{
    showDebug();
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());

    /* The first character must be i */
    if(*data.cbegin() != 'i' || *(data.cend() - 1) != 'e') {
        throw -1;
    }
    /* Optimized by compiler, so std::move is not necessary */
    ret = data.mid(1,data.size() - 2);
}

void BtQt::BtDecodeBencodeString(QByteArray const &data, QByteArray &ret)
{
    showDebug();
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());

    int colonPos = data.indexOf(':');
    ret = data.mid(colonPos + 1);

    if(!fastDecode) {
        if(data.mid(0, colonPos).toInt() != ret.size()) {
            qDebug() << "[Bencode] String: check failed!";
            throw -1;
        }
    }
}

/* get the last 'e' of a dictionary or list */
static inline int getLastE(QByteArray const &data, int pos)
{
    int eN = 1, ePos = pos;
    int lastEPos = ePos;
    while(eN != 0 && ePos < data.size()) {
        ++ ePos;
        if(data[ePos] == ':') {
            int skip = data.mid(lastEPos + 1, ePos - lastEPos - 1).toInt();
            ePos += skip;
            lastEPos = ePos;
        }
        else if (data[ePos] == 'e') {
            eN --;
            lastEPos = ePos;
        }
        else if(data[ePos] == 'i' || data[ePos] == 'l'||
                data[ePos] == 'd') {
            eN ++;
        }
    }
    if(ePos >= data.size()) return -1;
    return ePos;
}

void BtQt::BtDecodeBencodeList(QByteArray const &data, QList<QVariant> &ret)
{
    showDebug();
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());


    /* The first character must be l */
    if(*data.cbegin() != 'l' || *(data.cend() - 1) != 'e') {
        throw -1;
    }

    int pos = 1, nPos = 0;
    while(true) {
        QByteArray v;
        nPos = pickAndDecode(data, pos, v);
        if(nPos == -1) { // check if 'l' or 'd'
            if(data[pos] == 'l' || data[pos] == 'd') {
                // find correspond 'e'
                int ePos = getLastE(data, pos);
                if(ePos == -1) {
                    qDebug() << "Cannot get last e of a list!";
                    qDebug() << "List:" << data.mid(pos);
                    throw -1;
                }
                nPos = ePos + 1;
                if(data[pos] == 'l') {
                    QList<QVariant> vList;
                    try {
                        BtDecodeBencodeList(data.mid(pos, ePos - pos + 1), vList);
                        ret.push_back(QVariant(vList));
                    } catch (std::exception e) {
                        qDebug() << "Bencode List decode error, the input is: " << data.mid(pos, ePos - pos + 1);
                        return;
                    }
                } else {
                    QMap<QString, QVariant> vMap;
                    try {
                        BtDecodeBencodeDictionary(data.mid(pos, ePos - pos + 1), vMap);
                        ret.push_back(QVariant(vMap));
                    } catch (std::exception e) {
                        qDebug() << "Bencode Map decode error, the input is: " << data.mid(pos, ePos - pos + 1);
                    }
                }
            } else {
                throw -1;
            }
        } else {
            ret.push_back(QVariant(v));
        }
        pos = nPos;
        /* The end */
        if(pos >= (int)data.size() - 1) break;
    }
}

void BtQt::BtDecodeBencodeDictionary(QByteArray const &data, QMap<QString, QVariant> &ret)
{
    showDebug();
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());

    /* The first character must be d */
    if(*data.cbegin() != 'd' || *(data.cend() - 1) != 'e') {
        qDebug() << "[Bencode] Dictionary: the first or last character is not d or e, it's " << *data.cbegin() << *(data.cend() - 1);
        throw -1;
    }

    int pos = 1, nPos = 0;
    while(true) {
        /* Get key */
        QByteArray key;
        nPos = pickAndDecode(data, pos, key);
        if(nPos == -1 || nPos >= data.size() - 1) {
            /* This is not possible because all keys needs to be string.
             * And there must be a value */
            qDebug() << "All keys must be string. And there must be a value! nPos is " << nPos;
            qDebug() << "The rests are:";
            qDebug() << data.mid(pos);
            throw -1;
        }
        pos = nPos;

        /* Get Value */
        QByteArray v;
        nPos = pickAndDecode(data, pos, v);
        if(nPos == -1) { // check if 'l' or 'd'
            if(data[pos] == 'l' || data[pos] == 'd') {
                // find correspond 'e'
                int ePos = getLastE(data, pos);
                if(ePos == -1) {
                    qDebug() << "Cannot get last e of a dictionary!";
                    qDebug() << "Dictionary:" << data.mid(pos);
                    throw -1;
                }
                nPos = ePos + 1;
                if(data[pos] == 'l') {
                    QList<QVariant> vList;
                    try {
                        BtDecodeBencodeList(data.mid(pos, ePos - pos + 1), vList);
                        ret.insert(key, QVariant(vList));
                    } catch (std::exception e) {
                        qDebug() << "Bencode List decode error, the input is: " << data.mid(pos, ePos - pos + 1);
                        return;
                    }
                } else {
                    QMap<QString, QVariant> vMap;
                    try {
                        BtDecodeBencodeDictionary(data.mid(pos, ePos - pos + 1), vMap);
                        ret.insert(key, QVariant(vMap));
                    } catch (std::exception e) {
                        qDebug() << "Bencode Map decode error, the input is: " << data.mid(pos, ePos - pos + 1);
                    }
                }
            } else {
                qDebug() << "The contents of dictionary is broken!";
                qDebug() << "Broken data is started with:";
                qDebug() << data.mid(pos, 10);
                throw -1;
            }
        } else {
            ret.insert(key, QVariant(v));
        }
        pos = nPos;
        /* The end */
        if(pos >= (int)data.size() - 1) break;
    }
}
