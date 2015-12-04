#pragma once

#ifndef __BTDEFS_H__
#define __BTDEFS_H__

#include <QString>

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

#define NAMESPACE_BEGIN(X) namespace X {
#define NAMESPACE_END(X) }

#endif
