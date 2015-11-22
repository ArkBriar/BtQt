#pragma once

#ifndef __BTQT_H__
#define __BTQT_H__

#include <BtTorrent.h>
#include <BtTracker.h>
#include <BtDebug.h>
#include <BtBencode.h>
#include <BtPeer.h>
#include <BtCore.h>

namespace BtQt {
    const QString application = "BtQt";
    const QString version = "0.1";

    // a.b.c -> a(b)c, 'a' and 'c' take 1 bit
    const QString versionInString = "0010";
    const QString AzureusClientId = "QB";
}

#ifndef BT_NO_DEPRECATED_FUNCTION
#define BT_NO_DEPRECATED_FUNCTION
#endif // BT_NO_DEPRECATED_FUNCTION

#endif // __BTQT_H__
