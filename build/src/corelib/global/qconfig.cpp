/* This file is used to generate the Qt configuration info for the Core library.
 * The 'qt_generate_qconfig_cpp' cmake routine
 * contains variables that replace '@' entries in this file. It's important to
 * align these values with the following:
 *
 * - QLibraryInfo::LibraryPath enum in qtbase/src/corelib/global/qlibraryinfo.h
 * - qtConfEntries in qtbase/src/corelib/global/qlibraryinfo.cpp
 */
#include "private/qoffsetstringarray_p.h"

/* Installation Info */
static const char qt_configure_prefix_path_str  [12+256] = "qt_prfxpath=";

static constexpr auto qt_configure_strs = QT_PREPEND_NAMESPACE(qOffsetStringArray)(
    R"qconfig(./doc)qconfig",
    R"qconfig(include)qconfig",
    R"qconfig(lib)qconfig",
    R"qconfig(./libexec)qconfig",
    R"qconfig(bin)qconfig",
    R"qconfig(./plugins)qconfig",
    R"qconfig(./qml)qconfig",
    R"qconfig(.)qconfig",
    R"qconfig(.)qconfig",
    R"qconfig(./translations)qconfig",
    R"qconfig(examples)qconfig",
    R"qconfig(tests)qconfig"
);

#define QT_CONFIGURE_SETTINGS_PATH "etc/xdg"
#define QT_CONFIGURE_LIBLOCATION_TO_PREFIX_PATH "../"_L1
#define QT_CONFIGURE_PREFIX_PATH qt_configure_prefix_path_str + 12
