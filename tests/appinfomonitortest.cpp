// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStringList>
#include <QHash>

#include "appinfomonitor.h"

// fromGStrV is defined in appinfomonitor.cpp with external linkage but not
// declared in any header; provide an extern declaration so tests can call it.
extern QStringList fromGStrV(const char * const * gstr_array);

class TestAppInfoMonitor : public QObject
{
    Q_OBJECT
private slots:
    void constructAndDestruct();
    void allAppInfosShouldBeShownReturnsList();
    void allAppInfosContainsFixtureApps();
    void allAppInfosContainsVendorApp();
    void fromGStrVWithNull();
    void fromGStrVWithValidArray();
    void fromGStrVWithEmptyArray();
    void onAppInfoMonitorChangedEmitsChanged();
    void slotOnAppInfoMonitorChangedEmitsChanged();
};

void TestAppInfoMonitor::constructAndDestruct()
{
    {
        AppInfoMonitor monitor(nullptr);
        QVERIFY(monitor.m_appInfoMonitor != nullptr);
    }
}

void TestAppInfoMonitor::allAppInfosShouldBeShownReturnsList()
{
    AppInfoMonitor monitor(nullptr);
    QList<QHash<QString, QString>> results = monitor.allAppInfosShouldBeShown();
    QVERIFY(results.size() > 0);
}

void TestAppInfoMonitor::allAppInfosContainsFixtureApps()
{
    AppInfoMonitor monitor(nullptr);
    QList<QHash<QString, QString>> results = monitor.allAppInfosShouldBeShown();

    bool found = false;
    for (const auto &app : results) {
        if (app.value("id").contains("dde-file-manager")) {
            found = true;
            QVERIFY(app.contains("displayname"));
            QVERIFY(app.contains("name"));
            QVERIFY(app.contains("id"));
            QVERIFY(app.contains("filepath"));
            QVERIFY(app.contains("categories"));
            QVERIFY(app.contains("keywords"));
            QVERIFY(app.contains("icon"));
            break;
        }
    }
    QVERIFY(found);
}

void TestAppInfoMonitor::allAppInfosContainsVendorApp()
{
    AppInfoMonitor monitor(nullptr);
    QList<QHash<QString, QString>> results = monitor.allAppInfosShouldBeShown();

    bool found = false;
    for (const auto &app : results) {
        if (app.value("id").contains("dde-test-vendor")) {
            found = true;
            // X-Deepin-Vendor=deepin + non-empty GenericName => displayName = genericName
            QCOMPARE(app.value("displayname"), QStringLiteral("Test Generic"));
            QVERIFY(!app.value("icon").isEmpty());
            QVERIFY(app.value("keywords").contains("test"));
            break;
        }
    }
    QVERIFY(found);
}

void TestAppInfoMonitor::fromGStrVWithNull()
{
    QStringList result = fromGStrV(nullptr);
    QVERIFY(result.isEmpty());
}

void TestAppInfoMonitor::fromGStrVWithValidArray()
{
    const char *const array[] = {"hello", "world", nullptr};
    QStringList result = fromGStrV(array);
    QCOMPARE(result.size(), 2);
    QCOMPARE(result[0], QStringLiteral("hello"));
    QCOMPARE(result[1], QStringLiteral("world"));
}

void TestAppInfoMonitor::fromGStrVWithEmptyArray()
{
    const char *const array[] = {nullptr};
    QStringList result = fromGStrV(array);
    QVERIFY(result.isEmpty());
}

void TestAppInfoMonitor::onAppInfoMonitorChangedEmitsChanged()
{
    AppInfoMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &AppInfoMonitor::changed);

    monitor.onAppInfoMonitorChanged(nullptr);

    QCOMPARE(spy.count(), 1);
}

void TestAppInfoMonitor::slotOnAppInfoMonitorChangedEmitsChanged()
{
    AppInfoMonitor monitor(nullptr);
    QSignalSpy spy(&monitor, &AppInfoMonitor::changed);

    AppInfoMonitor::slot_onAppInfoMonitorChanged(nullptr, &monitor);

    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestAppInfoMonitor)
#include "appinfomonitortest.moc"
