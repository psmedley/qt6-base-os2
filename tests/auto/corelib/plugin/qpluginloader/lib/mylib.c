// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <qglobal.h>

#if defined(Q_CC_MSVC) || defined(Q_CC_MSVC_NET) || defined(Q_CC_BOR) || defined(Q_OS_OS2)
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT
#endif

#if defined(Q_CC_BOR)
# define BORLAND_STDCALL __stdcall
#elif defined(Q_OS_OS2)
# define BORLAND_STDCALL _System
#else
# define BORLAND_STDCALL
#endif

static int pluginVariable = 0xc0ffee;
LIB_EXPORT int * BORLAND_STDCALL pointerAddress()
{
    return &pluginVariable;
}

LIB_EXPORT int BORLAND_STDCALL version()
{
    return 1;
}

