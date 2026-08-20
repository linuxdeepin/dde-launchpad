// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QLoggingCategory>

#include "appinfo.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")
}

class TestGioAppInfo: public QObject
{
    Q_OBJECT
    
public:
    TestGioAppInfo() {
        qCDebug(logTest) << "Constructing TestGioAppInfo test object";
    }
    
    ~TestGioAppInfo() {
        qCDebug(logTest) << "Destroying TestGioAppInfo test object";
    }
    
private slots:
    void dfmInfo();
};

void TestGioAppInfo::dfmInfo()
{
    qCInfo(logTest) << "Testing AppInfo::fullPathByDesktopId with dde-file-manager.desktop";
    QString path = AppInfo::fullPathByDesktopId("dde-file-manager.desktop");
    qCDebug(logTest) << "Retrieved path:" << path;
    
    QVERIFY(QFile::exists(path));
    qCDebug(logTest) << "Verified that file exists at path:" << path;
    
    QVERIFY(!path.isEmpty());
    qCInfo(logTest) << "Verified path is not empty, test passed";
}

QTEST_MAIN(TestGioAppInfo)
#include "gioappinfotest.moc"
