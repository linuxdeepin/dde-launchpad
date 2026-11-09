// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>

#include "appinfo.h"

class TestGioAppInfo: public QObject
{
    Q_OBJECT
private slots:
    void dfmInfo();
    void fullPathByDesktopIdWithInvalidId();
    void launchByDesktopIdWithValidId();
    void launchByDesktopIdWithInvalidId();
};

// The desktop file is provided through XDG_DATA_HOME, which ctest sets to
// tests/gio-fixture (see tests/CMakeLists.txt); set it before the process starts
// because GIO resolves and caches its XDG directories early.
void TestGioAppInfo::dfmInfo()
{
    const QString path = AppInfo::fullPathByDesktopId("dde-file-manager.desktop");

    QVERIFY(!path.isEmpty());
    QVERIFY(QFile::exists(path));
    QVERIFY(path.endsWith("dde-file-manager.desktop"));
}

void TestGioAppInfo::fullPathByDesktopIdWithInvalidId()
{
    const QString path = AppInfo::fullPathByDesktopId("nonexistent-app-12345.desktop");
    QVERIFY(path.isEmpty());
}

// The fixture desktop file has Exec=/bin/true, so launching is safe.
void TestGioAppInfo::launchByDesktopIdWithValidId()
{
    bool result = AppInfo::launchByDesktopId("dde-file-manager.desktop");
    QVERIFY(result);
}

void TestGioAppInfo::launchByDesktopIdWithInvalidId()
{
    bool result = AppInfo::launchByDesktopId("nonexistent-app-12345.desktop");
    QVERIFY(!result);
}

QTEST_MAIN(TestGioAppInfo)
#include "gioappinfotest.moc"
