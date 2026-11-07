/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#define QT_NO_URL_CAST_FROM_STRING

#include "qos2services.h"
#include <QtCore/qurl.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qstring.h>

#ifndef ERROR_SMG_START_IN_BACKGROUND
#define ERROR_SMG_START_IN_BACKGROUND 457
#endif

QT_BEGIN_NAMESPACE

namespace {

enum LaunchType {
    LaunchBrowser,  // target is an URL to pass to the default browser
    LaunchMail,     // target is a mailto: URL to pass to the default mail app
    LaunchDocument, // target is a native path to open via its WPS object
    LaunchProgram,  // target is a native path of an executable to start
};

// Reads a string from the WPURLDEFAULTSETTINGS application of OS2.INI into buf (always
// NUL-terminated). Returns false if there is no such key or it's empty.
bool queryUrlSetting(const char *key, char *buf, ULONG size)
{
    buf[0] = '\0';
    ULONG sz = size;
    if (!PrfQueryProfileData(HINI_USERPROFILE, "WPURLDEFAULTSETTINGS", key, buf, &sz) || !sz) {
        buf[0] = '\0';
        return false;
    }
    buf[qMin(sz, size - 1)] = '\0';
    return buf[0] != '\0';
}

bool launch(const QByteArray &target, LaunchType type)
{
    if (target.isEmpty())
        return false;

    if (type == LaunchDocument) {
        HOBJECT hObject = WinQueryObject(target.constData());
        if (hObject != NULLHANDLE && WinOpenObject(hObject, 0 /*OPEN_DEFAULT*/, TRUE))
            return true;
        qWarning("WinOpenObject failed for \"%s\"", target.constData());
        return false;
    }

    CHAR defaultExe[CCHMAXPATH];
    QByteArray params;
    QString origDir;
    bool dirChanged = false;

    if (type == LaunchBrowser || type == LaunchMail) {
        const bool browser = type == LaunchBrowser;
        if (!queryUrlSetting(browser ? "DefaultBrowserExe" : "DefaultMailExe",
                             defaultExe, sizeof(defaultExe))) {
            qWarning("%s", browser
                     ? "A default browser is not designated in WPURLDEFAULTSETTINGS->DefaultBrowserExe in OS2.INI"
                     : "A default mail app is not designated in WPURLDEFAULTSETTINGS->DefaultMailExe in OS2.INI");
            return false;
        }

        // Start the program in its working directory (DosStartSession has no parameter for that).
        CHAR defaultDir[CCHMAXPATH];
        if (queryUrlSetting(browser ? "DefaultWorkingDir" : "DefaultMailWorkingDir",
                            defaultDir, sizeof(defaultDir))) {
            origDir = QDir::currentPath();
            if (QDir::setCurrent(QFile::decodeName(defaultDir)))
                dirChanged = true;
            else
                qWarning("Cannot find the default working directory \"%s\"", defaultDir);
        }

        if (!browser) {
            CHAR mailParams[CCHMAXPATH];
            if (queryUrlSetting("DefaultMailParameters", mailParams, sizeof(mailParams))) {
                params = mailParams;
                params += ' ';
            }
        }
        params += target;
    }

    PID pid = 0;
    ULONG ulSessID = 0;
    CHAR achObjBuf[256] = {0};
    STARTDATA startData;

    memset(&startData, 0, sizeof(STARTDATA));
    startData.Length = sizeof(STARTDATA);
    startData.Related = SSF_RELATED_INDEPENDENT;
    startData.InheritOpt = SSF_INHERTOPT_PARENT;
    startData.FgBg = SSF_FGBG_FORE;
    startData.PgmControl = SSF_CONTROL_VISIBLE;
    startData.TraceOpt = SSF_TRACEOPT_NONE;
    startData.PgmTitle = NULL;
    startData.PgmName = type == LaunchProgram ? (PSZ)target.constData() : defaultExe;
    startData.TermQ = 0;
    startData.Environment = NULL;
    startData.PgmInputs = params.isEmpty() ? NULL : (PBYTE)params.constData();
    startData.SessionType = type == LaunchProgram ? SSF_TYPE_DEFAULT : SSF_TYPE_PM;
    startData.ObjectBuffer = achObjBuf;
    startData.ObjectBuffLen = (ULONG)sizeof(achObjBuf);

    APIRET rc = DosStartSession(&startData, &ulSessID, &pid);

    if (dirChanged) {
        // OS/2 keeps a current directory per drive: reset the one we changed to the drive's
        // root before going back to the original directory (which may be on another drive).
        QString root = QDir::currentPath();
        root.truncate(3);
        QDir::setCurrent(root);
        if (!QDir::setCurrent(origDir))
            qWarning("Resetting the current directory failed");
    }

    // ERROR_SMG_START_IN_BACKGROUND just means we couldn't bring the new session to front.
    if (rc != NO_ERROR && rc != ERROR_SMG_START_IN_BACKGROUND) {
        qWarning("DosStartSession failed with %lu", rc);
        return false;
    }

    return true;
}

} // unnamed namespace

bool QOS2Services::openUrl(const QUrl &url)
{
    if (!url.isValid())
        return false;
    const LaunchType type = url.scheme() == u"mailto" ? LaunchMail : LaunchBrowser;
    return launch(url.toEncoded(), type);
}

bool QOS2Services::openDocument(const QUrl &url)
{
    // Note: use toLocalFile() to get rid of the scheme and of the percent encoding (otherwise
    // e.g. paths with spaces won't be found by WinQueryObject); WinQueryObject also requires
    // backslashes.
    const QString path = QDir::toNativeSeparators(url.toLocalFile());
    if (path.isEmpty())
        return false;

    const bool isExe = QFileInfo(path).suffix().compare(QLatin1String("exe"), Qt::CaseInsensitive) == 0;
    return launch(QFile::encodeName(path), isExe ? LaunchProgram : LaunchDocument);
}

QT_END_NAMESPACE
