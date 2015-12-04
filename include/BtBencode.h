#pragma once

#ifndef __BTBENCODE_H__
#define __BTBENCODE_H__

#include <QByteArray>
#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>
#include <BtDefs.h>

/* First, let`s show some facts about bencode
 * From wikipedia: [https://en.wikipedia.org/wiki/Bencode]
 * * An integer is encoded as i<integer encoded in base ten ASCII>e. (Negative zero is not allowed)
 *   - 42 is `i42e`, -42 is `i-42e`, 0 is `i0e`
 *
 * * A byte string (a sequence of bytes, not necessarily characters) is encodes as <length>:<contents>. The length is encoded in base 10 (non-negative)
 *   - spam is `4:spam`, network is `7:network`
 *
 * * A list of values is encoded as l<contents>e. The <contents> consist of the bencoded elements of the list, in order, concatenated.
 *   - A list consisting of the string "spam" and the number 42 would be encoded as: `l4:spami42ee`
 *
 * * A dictionary is encoded as d<contents>e. Like list, <contents> is encoded in bencode and concatenated.
 *   - dictionary `{"bar":"spam", "foo":42}` would be encoded as follows: `d3:bar4:spam3:fooi42ee`
 * */

NAMESPACE_BEGIN(BtQt)

/* Provide two sets of functions to decode from the torrent data
 * These functions will throw exceptions if error occurs
 * */
void BtDecode(QByteArray const &data, QVariant &);

void BtDecodeBencodeInteger(QByteArray const &data, QByteArray &);

void BtDecodeBencodeString(QByteArray const &data, QByteArray &);

void BtDecodeBencodeList(QByteArray const &data, QList<QVariant> &);

void BtDecodeBencodeDictionary(QByteArray const &data, QMap<QString, QVariant> &);

/* Provide two sets of functions to encode data to the torrent data
 * These functions will throw exceptions if error occurs
 * */
void BtEncodeBencodeInteger(qint64 data, QByteArray &);

void BtEncodeBencodeString(QByteArray const &, QByteArray &);
/* Contents(what in QVariant) must be one of:
 * Int, String(ByteArray), List<Contents>, Map<String, Contents>
 * Due to QVariant's problem, when canConvert QByteArray, it will check if
 * the value can convert to int64. If possible, it will call encode as
 * integer instead of string
 * */
void BtEncodeBencodeList(QList<QVariant> const &, QByteArray &);

void BtEncodeBencodeMap(QMap<QString, QVariant> const&, QByteArray &);

NAMESPACE_END(BtQt)

#endif // __BTBENCODE_H__
