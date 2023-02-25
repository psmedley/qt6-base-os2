/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/qtest.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qplugin.h>
#include <private/qfactoryloader_p.h>
#include "plugin1/plugininterface1.h"
#include "plugin2/plugininterface2.h"

#if !QT_CONFIG(library)
Q_IMPORT_PLUGIN(Plugin1)
Q_IMPORT_PLUGIN(Plugin2)
#endif

class tst_QFactoryLoader : public QObject
{
    Q_OBJECT

#ifdef Q_OS_ANDROID
    QSharedPointer<QTemporaryDir> directory;
#endif

    QString binFolder;
public slots:
    void initTestCase();

private slots:
    void usingTwoFactoriesFromSameDir();
    void extraSearchPath();
};

#define BIN_FOLDER "bin"
static const char *binFolderC = BIN_FOLDER;

void tst_QFactoryLoader::initTestCase()
{
    // On Android the plugins are bundled into APK's libs subdir
#ifndef Q_OS_ANDROID
    binFolder = QFINDTESTDATA(binFolderC);
    // Support running the test case from the build dir w/o installing.
    if (binFolder.isEmpty())
        binFolder = QFINDTESTDATA("../" BIN_FOLDER);
    QVERIFY2(!binFolder.isEmpty(), "Unable to locate '" BIN_FOLDER "' folder");
#endif
}

void tst_QFactoryLoader::usingTwoFactoriesFromSameDir()
{
#if QT_CONFIG(library) && !defined(Q_OS_ANDROID)
    // set the library path to contain the directory where the 'bin' dir is located
    QCoreApplication::setLibraryPaths( { QFileInfo(binFolder).absolutePath() });
#endif

    const QString suffix = QLatin1Char('/') + QLatin1String(binFolderC);
    QFactoryLoader loader1(PluginInterface1_iid, suffix);

    PluginInterface1 *plugin1 = qobject_cast<PluginInterface1 *>(loader1.instance(0));
    QVERIFY2(plugin1,
             qPrintable(QString::fromLatin1("Cannot load plugin '%1'")
                        .arg(QLatin1String(PluginInterface1_iid))));

    QFactoryLoader loader2(PluginInterface2_iid, suffix);

    PluginInterface2 *plugin2 = qobject_cast<PluginInterface2 *>(loader2.instance(0));
    QVERIFY2(plugin2,
             qPrintable(QString::fromLatin1("Cannot load plugin '%1'")
                        .arg(QLatin1String(PluginInterface2_iid))));

    QCOMPARE(plugin1->pluginName(), QLatin1String("Plugin1 ok"));
    QCOMPARE(plugin2->pluginName(), QLatin1String("Plugin2 ok"));
}

void tst_QFactoryLoader::extraSearchPath()
{
#if defined(Q_OS_ANDROID) && !QT_CONFIG(library)
    QSKIP("Test not applicable in this configuration.");
#else
#ifdef Q_OS_ANDROID
    // On Android the libs are not stored in binFolder, but bundled into
    // APK's libs subdir
    const QStringList androidLibsPaths = QCoreApplication::libraryPaths();
    QCOMPARE(androidLibsPaths.size(), 1);
#endif
    QCoreApplication::setLibraryPaths(QStringList());

#ifndef Q_OS_ANDROID
    QString pluginsPath = QFileInfo(binFolder).absoluteFilePath();
    QFactoryLoader loader1(PluginInterface1_iid, "/nonexistent");
#else
    QString pluginsPath = androidLibsPaths.first();
    // On Android we still need to specify a valid suffix, because it's a part
    // of a file name, not directory structure
    const QString suffix = QLatin1Char('/') + QLatin1String(binFolderC);
    QFactoryLoader loader1(PluginInterface1_iid, suffix);
#endif

    // it shouldn't have scanned anything because we haven't given it a path yet
    QVERIFY(loader1.metaData().isEmpty());

    loader1.setExtraSearchPath(pluginsPath);
    PluginInterface1 *plugin1 = qobject_cast<PluginInterface1 *>(loader1.instance(0));
    QVERIFY2(plugin1,
             qPrintable(QString::fromLatin1("Cannot load plugin '%1'")
                        .arg(QLatin1String(PluginInterface1_iid))));

    QCOMPARE(plugin1->pluginName(), QLatin1String("Plugin1 ok"));

    // check that it forgets that plugin
    loader1.setExtraSearchPath(QString());
    QVERIFY(loader1.metaData().isEmpty());
#endif
}

QTEST_MAIN(tst_QFactoryLoader)
#include "tst_qfactoryloader.moc"
