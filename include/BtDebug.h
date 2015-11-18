#pragma once

/* Macros for debug */
#ifndef QT_NO_DEBUG
#ifdef __GNUC__
#define function_debug() \
    qDebug() << "[DEBUG] " << __PRETTY_FUNCTION__ << __LINE__
#else
#define function_debug() \
    qDebug() << "[DEBUG] " << __FUNCTION__ << __LINE__
#endif

#else
#define function_debug()
#endif
