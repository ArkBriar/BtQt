#include "BtBencode.h"
#include <QtGlobal>
#include <QStringList>
#include <QDebug>

using namespace BtQt;

/* if fastDecode turns to true, decode functions below will skip
 * some necessary check */
static bool fastDecode = false;

/* The two functions below do not handle 'd' and 'l'
 * if error occurs or reach end, return -1 */
static inline int pickAndDecode(QString const &data, int pos, QString &ret) {
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

static inline int pickAndDecode(std::string const &data, int pos, QString &ret) {
    return pickAndDecode(QString::fromStdString(data), pos, ret);
/*
 *    if(pos >= (int)data.size()) return -1;
 *    if(data[pos] != 'i' && data[pos] != 'l' && data[pos] != 'd') {
 *        // is string
 *        int colonPos = data.find(':', pos);
 *        int len = stoi(data.substr(pos, colonPos - pos));
 *        try {
 *            BtDecodeBencodeString(data.substr(pos, colonPos - pos + len + 1), ret);
 *        } catch (std::exception e){
 *            qDebug() << "Bencode string decode error, the input is: " << QString::fromStdString(data.substr(pos, colonPos - pos + len + 1));
 *            goto PICK_AND_DECODE_ERROR;
 *        }
 *        return colonPos + len + 1;
 *    } else if(data[pos] == 'i') {
 *        // is integer
 *        int ePos = data.find('e', pos);
 *        try {
 *            BtDecodeBencodeInteger(data.substr(pos, ePos - pos + 1), ret);
 *        } catch (std::exception e) {
 *            qDebug() << "Bencode integer decode error, the input is: " << QString::fromStdString(data.substr(pos, ePos - pos + 1));
 *            goto PICK_AND_DECODE_ERROR;
 *        }
 *        return ePos + 1;
 *    }
 *
 *PICK_AND_DECODE_ERROR:
 *    return -1;
 */
}

void BtQt::BtDecodeBencodeInteger(std::string const &data, QString &ret)
{
    /*
     *BtDecodeBencodeInteger(QString::fromStdString(data), ret);
     */
    Q_ASSERT(ret.isEmpty() && !data.empty());

    /* The first character must be i */
    if(data.front() == 'i' && data.back() == 'e') {
        throw -1;
    }
    ret = QString::fromStdString(data.substr(1, data.size() - 2));
}

void BtQt::BtDecodeBencodeInteger(QString const &data, QString &ret)
{
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());

    /* The first character must be i */
    if(*data.cbegin() == 'i' && *data.cend() == 'e') {
        throw -1;
    }
    /* Optimized by compiler, so std::move is not necessary */
    ret = data.mid(1,data.size() - 2);
}

void BtQt::BtDecodeBencodeString(std::string const &data, QString &ret)
{
    /*
     *BtDecodeBencodeString(QString::fromStdString(data), ret);
     */
    Q_ASSERT(ret.isEmpty() && !data.empty());

    size_t colonPos = data.find(':');
    ret = QString::fromStdString(data.substr(colonPos + 1));

    if(!fastDecode) {
        if(ret.size() != std::stoi(data.substr(0, colonPos))) {
            throw -1;
        }
    }
}

void BtQt::BtDecodeBencodeString(QString const &data, QString &ret)
{
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());

    QStringList list = data.split(':');
    if(list.size() != 2) { // there must be two strings in the list
        throw -1;
    }
    ret = *list.cend();

    if(!fastDecode) {
        if(list.cbegin()->toInt() != ret.size()) {
            throw -1;
        }
    }
}

void BtQt::BtDecodeBencodeList(std::string const &data, QList<QVariant> &ret)
{
    /*
     *BtDecodeBencodeList(QString::fromStdString(data), ret);
     */
    Q_ASSERT(ret.isEmpty() && !data.empty());


    /* The first character must be l */
    if(data.front() == 'l' && data.back() == 'e') {
        throw -1;
    }

    int pos = 0, nPos = 0;
    while(true) {
        QString v;
        nPos = pickAndDecode(data, pos, v);
        if(nPos == -1) { // check if 'l' or 'd'
            if(data[pos] == 'l' && data[pos] == 'd') {
                // find correspond 'e'
                int eN = 1, ePos = pos;
                while(eN != 0) {
                    ++ ePos;
                    if(data[ePos] == 'e') eN --;
                    else if(data[ePos] == 'i' || data[ePos] == 'l'||
                            data[ePos] == 'd') eN ++;
                }
                nPos = ePos + 1;
                if(data[pos] == 'l') {
                    QList<QVariant> vList;
                    try {
                        BtDecodeBencodeList(data.substr(pos, ePos), vList);
                        ret.push_back(QVariant(vList));
                    } catch (std::exception e) {
                        qDebug() << "Bencode List decode error, the input is: " << QString::fromStdString(data.substr(pos, ePos - pos + 1));
                        return;
                    }
                } else {
                    QMap<QString, QVariant> vMap;
                    try {
                        BtDecodeBencodeDictionary(data.substr(pos, ePos), vMap);
                        ret.push_back(QVariant(vMap));
                    } catch (std::exception e) {
                        qDebug() << "Bencode Map decode error, the input is: " << QString::fromStdString(data.substr(pos, ePos - pos + 1));
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
        if(pos > (int)data.size() - 1) break;
    }

}

void BtQt::BtDecodeBencodeList(QString const &data, QList<QVariant> &ret)
{
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());


    /* The first character must be l */
    if(*data.cbegin() == 'l' && *data.cend() == 'e') {
        throw -1;
    }

    int pos = 0, nPos = 0;
    while(true) {
        QString v;
        nPos = pickAndDecode(data, pos, v);
        if(nPos == -1) { // check if 'l' or 'd'
            if(data[pos] == 'l' && data[pos] == 'd') {
                // find correspond 'e'
                int eN = 1, ePos = pos;
                while(eN != 0) {
                    ++ ePos;
                    if(data[ePos] == 'e') eN --;
                    else if(data[ePos] == 'i' || data[ePos] == 'l'||
                            data[ePos] == 'd') eN ++;
                }
                nPos = ePos + 1;
                if(data[pos] == 'l') {
                    QList<QVariant> vList;
                    try {
                        BtDecodeBencodeList(data.mid(pos, ePos), vList);
                        ret.push_back(QVariant(vList));
                    } catch (std::exception e) {
                        qDebug() << "Bencode List decode error, the input is: " << data.mid(pos, ePos - pos + 1);
                        return;
                    }
                } else {
                    QMap<QString, QVariant> vMap;
                    try {
                        BtDecodeBencodeDictionary(data.mid(pos, ePos), vMap);
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
        if(pos >= (int)data.size()) break;
    }
}

void BtQt::BtDecodeBencodeDictionary(std::string const &data, QMap<QString, QVariant> &ret)
{
    BtDecodeBencodeDictionary(QString::fromStdString(data), ret);
/*
 *    Q_ASSERT(ret.isEmpty() && !data.empty());
 *
 *     The first character must be d
 *    if(data.front() == 'd' && data.back() == 'e') {
 *        throw -1;
 *    }
 */
}
void BtQt::BtDecodeBencodeDictionary(QString const &data, QMap<QString, QVariant> &ret)
{
    Q_ASSERT(ret.isEmpty() && !data.isEmpty());

    /* The first character must be d */
    if(*data.cbegin() == 'd' && *data.cend() == 'e') {
        throw -1;
    }

    int pos = 0, nPos = 0;
    while(true) {
        /* Get key */
        QString key;
        nPos = pickAndDecode(data, pos, key);
        if(nPos == -1 || nPos >= data.size()) {
            /* This is not possible because all keys needs to be string.
             * And there must be a value */
            throw -1;
        }
        pos = nPos;

        /* Get Value */
        QString v;
        nPos = pickAndDecode(data, pos, v);
        if(nPos == -1) { // check if 'l' or 'd'
            if(data[pos] == 'l' && data[pos] == 'd') {
                // find correspond 'e'
                int eN = 1, ePos = pos;
                while(eN != 0) {
                    ++ ePos;
                    if(data[ePos] == 'e') eN --;
                    else if(data[ePos] == 'i' || data[ePos] == 'l'||
                            data[ePos] == 'd') eN ++;
                }
                nPos = ePos + 1;
                if(data[pos] == 'l') {
                    QList<QVariant> vList;
                    try {
                        BtDecodeBencodeList(data.mid(pos, ePos), vList);
                        ret.insert(key, QVariant(vList));
                    } catch (std::exception e) {
                        qDebug() << "Bencode List decode error, the input is: " << data.mid(pos, ePos - pos + 1);
                        return;
                    }
                } else {
                    QMap<QString, QVariant> vMap;
                    try {
                        BtDecodeBencodeDictionary(data.mid(pos, ePos), vMap);
                        ret.insert(key, QVariant(vMap));
                    } catch (std::exception e) {
                        qDebug() << "Bencode Map decode error, the input is: " << data.mid(pos, ePos - pos + 1);
                    }
                }
            } else {
                throw -1;
            }
        } else {
            ret.insert(key, QVariant(v));
        }
        pos = nPos;
        /* The end */
        if(pos >= (int)data.size()) break;
    }
}
