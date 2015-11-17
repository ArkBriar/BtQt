#pragma once

#ifndef __BTBENCODE_H__
#define __BTBENCODE_H__

/* This package supports two forms of input data:
 * STL and Qt
 * */
/* STL header files*/
#include <cstring>
/* Qt header files */
#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

/* First, let`s show some facts about bencode
 * From wikipedia: [https://en.wikipedia.org/wiki/Bencode]
 * * An integer is encoded as i<integer encoded in base ten ASCII>e. (Negative zero is not allowed)
 *   - 42 is `i42e`, -42 is `i-42e`, 0 is `i0e`
 * * A byte string (a sequence of bytes, not necessarily characters) is encodes as <length>:<contents>. The length is encoded in base 10 (non-negative)
 *   - spam is `4:spam`, network is `7:network`
 * * A list of values is encoded as l<contents>e. The <contents> consist of the bencoded elements of the list, in order, concatenated.
 *   - A list consisting of the string "spam" and the number 42 would be encoded as: `l4:spami42ee`
 * * A dictionary is encoded as d<contents>e. Like list, <contents> is encoded in bencode and concatenated.
 *   - dictionary `{"bar":"spam", "foo":42}` would be encoded as follows: `d3:bar4:spam3:fooi42ee`
 * */

namespace BtQt {
    /* Provide two sets of functions to decode from the torrent data
     * These functions will throw exceptions if error occurs
     * */
    /*
     *void BtDecode(std::string &torrentData);
     *void BtDecode(QString &torrentData);
     */

    void BtDecodeBencodeInteger(std::string const &data, QString &);
    void BtDecodeBencodeInteger(QString const &data, QString &);

    void BtDecodeBencodeString(std::string const &data, QString &);
    void BtDecodeBencodeString(QString const &data, QString &);

    void BtDecodeBencodeList(std::string const &data, QList<QVariant> &);
    void BtDecodeBencodeList(QString const &data, QList<QVariant> &);

    void BtDecodeBencodeDictionary(std::string const &data, QMap<QString, QVariant> &);
    void BtDecodeBencodeDictionary(QString const &data, QMap<QString, QVariant> &);

    /* Provide two sets of functions to encode data to the torrent data
     * These functions will throw exceptions if error occurs
     * */
    /*
     *void BtEncode(std::string &torrentData);
     *void BtEncode(QString &torrentData);
     */
    /* Not implemented */
}

#endif // __BTBENCODE_H__
