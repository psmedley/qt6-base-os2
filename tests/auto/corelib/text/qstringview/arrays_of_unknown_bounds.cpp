// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "arrays_of_unknown_bounds.h"

const char string_array[] = "abc\0def";
const int string_array_size = 3;

#ifdef __cpp_char8_t
const char8_t u8string_array[] = u8"abc\0def";
const int u8string_array_size = 3;
#endif

const char16_t u16string_array[] = u"abc\0def";
const int u16string_array_size = 3;

const wchar_t wstring_array[] = L"abc\0def";
const int wstring_array_size = 3;
