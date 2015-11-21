#pragma once

#ifndef __BTQT_H__
#define __BTQT_H__

#include <BtTorrent.h>
#include <BtTracker.h>
#include <BtDebug.h>
#include <BtBencode.h>
#include <BtPeer.h>

namespace BtQt {
    const QString application = "BtQt";
    const QString version = "0.1 alpha";
}

#ifndef BT_NO_DEPRECATED_FUNCTION
#define BT_NO_DEPRECATED_FUNCTION
#endif // BT_NO_DEPRECATED_FUNCTION

#endif // __BTQT_H__
