// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef VS_TEST_P_H
#define VS_TEST_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qglobal_p.h>

#ifdef Q_OS_WIN

#include <qt_windows.h>

#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions:
//
// cbuffer buf
// {
//
//   row_major float4x4 ubuf_mvp;       // Offset:    0 Size:    64
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// buf                               cbuffer      NA          NA            cb0      1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// TEXCOORD                 0   xyzw        0     NONE   float   xyzw
// TEXCOORD                 1   xyz         1     NONE   float   xyz
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xyzw
// TEXCOORD                 0   xyz         1     NONE   float   xyz
//
vs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[4], immediateIndexed
dcl_input v0.xyzw
dcl_input v1.xyz
dcl_output_siv o0.xyzw, position
dcl_output o1.xyz
dcl_temps 1
mul r0.xyzw, v0.yyyy, cb0[1].xyzw
mad r0.xyzw, v0.xxxx, cb0[0].xyzw, r0.xyzw
mad r0.xyzw, v0.zzzz, cb0[2].xyzw, r0.xyzw
mad o0.xyzw, v0.wwww, cb0[3].xyzw, r0.xyzw
mov o1.xyz, v1.xyzx
ret
// Approximately 6 instruction slots used
#endif

inline constexpr BYTE g_testVertexShader[] =
{
     68,  88,  66,  67,  75, 198,
     18, 149, 172, 244, 247, 123,
     98,  31, 128, 185,  22, 199,
    182, 233,   1,   0,   0,   0,
    140,   3,   0,   0,   5,   0,
      0,   0,  52,   0,   0,   0,
     60,   1,   0,   0, 136,   1,
      0,   0, 224,   1,   0,   0,
    240,   2,   0,   0,  82,  68,
     69,  70,   0,   1,   0,   0,
      1,   0,   0,   0,  96,   0,
      0,   0,   1,   0,   0,   0,
     60,   0,   0,   0,   0,   5,
    254, 255,   0,   1,   0,   0,
    216,   0,   0,   0,  82,  68,
     49,  49,  60,   0,   0,   0,
     24,   0,   0,   0,  32,   0,
      0,   0,  40,   0,   0,   0,
     36,   0,   0,   0,  12,   0,
      0,   0,   0,   0,   0,   0,
     92,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      1,   0,   0,   0,   1,   0,
      0,   0,  98, 117, 102,   0,
     92,   0,   0,   0,   1,   0,
      0,   0, 120,   0,   0,   0,
     64,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
    160,   0,   0,   0,   0,   0,
      0,   0,  64,   0,   0,   0,
      2,   0,   0,   0, 180,   0,
      0,   0,   0,   0,   0,   0,
    255, 255, 255, 255,   0,   0,
      0,   0, 255, 255, 255, 255,
      0,   0,   0,   0, 117,  98,
    117, 102,  95, 109, 118, 112,
      0, 102, 108, 111,  97, 116,
     52, 120,  52,   0, 171, 171,
      2,   0,   3,   0,   4,   0,
      4,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0, 169,   0,   0,   0,
     77, 105,  99, 114, 111, 115,
    111, 102, 116,  32,  40,  82,
     41,  32,  72,  76,  83,  76,
     32,  83, 104,  97, 100, 101,
    114,  32,  67, 111, 109, 112,
    105, 108, 101, 114,  32,  49,
     48,  46,  49,   0,  73,  83,
     71,  78,  68,   0,   0,   0,
      2,   0,   0,   0,   8,   0,
      0,   0,  56,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
      0,   0,   0,   0,  15,  15,
      0,   0,  56,   0,   0,   0,
      1,   0,   0,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
      1,   0,   0,   0,   7,   7,
      0,   0,  84,  69,  88,  67,
     79,  79,  82,  68,   0, 171,
    171, 171,  79,  83,  71,  78,
     80,   0,   0,   0,   2,   0,
      0,   0,   8,   0,   0,   0,
     56,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0,
      3,   0,   0,   0,   0,   0,
      0,   0,  15,   0,   0,   0,
     68,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   1,   0,
      0,   0,   7,   8,   0,   0,
     83,  86,  95,  80, 111, 115,
    105, 116, 105, 111, 110,   0,
     84,  69,  88,  67,  79,  79,
     82,  68,   0, 171, 171, 171,
     83,  72,  69,  88,   8,   1,
      0,   0,  80,   0,   1,   0,
     66,   0,   0,   0, 106,   8,
      0,   1,  89,   0,   0,   4,
     70, 142,  32,   0,   0,   0,
      0,   0,   4,   0,   0,   0,
     95,   0,   0,   3, 242,  16,
     16,   0,   0,   0,   0,   0,
     95,   0,   0,   3, 114,  16,
     16,   0,   1,   0,   0,   0,
    103,   0,   0,   4, 242,  32,
     16,   0,   0,   0,   0,   0,
      1,   0,   0,   0, 101,   0,
      0,   3, 114,  32,  16,   0,
      1,   0,   0,   0, 104,   0,
      0,   2,   1,   0,   0,   0,
     56,   0,   0,   8, 242,   0,
     16,   0,   0,   0,   0,   0,
     86,  21,  16,   0,   0,   0,
      0,   0,  70, 142,  32,   0,
      0,   0,   0,   0,   1,   0,
      0,   0,  50,   0,   0,  10,
    242,   0,  16,   0,   0,   0,
      0,   0,   6,  16,  16,   0,
      0,   0,   0,   0,  70, 142,
     32,   0,   0,   0,   0,   0,
      0,   0,   0,   0,  70,  14,
     16,   0,   0,   0,   0,   0,
     50,   0,   0,  10, 242,   0,
     16,   0,   0,   0,   0,   0,
    166,  26,  16,   0,   0,   0,
      0,   0,  70, 142,  32,   0,
      0,   0,   0,   0,   2,   0,
      0,   0,  70,  14,  16,   0,
      0,   0,   0,   0,  50,   0,
      0,  10, 242,  32,  16,   0,
      0,   0,   0,   0, 246,  31,
     16,   0,   0,   0,   0,   0,
     70, 142,  32,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
     70,  14,  16,   0,   0,   0,
      0,   0,  54,   0,   0,   5,
    114,  32,  16,   0,   1,   0,
      0,   0,  70,  18,  16,   0,
      1,   0,   0,   0,  62,   0,
      0,   1,  83,  84,  65,  84,
    148,   0,   0,   0,   6,   0,
      0,   0,   1,   0,   0,   0,
      0,   0,   0,   0,   4,   0,
      0,   0,   4,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0
};

#endif // Q_OS_WIN

#endif // VS_TEST_P_H