// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLoggingCategory>

#include "../src/models/frequentlyusedproxymodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

constexpr int NameRole = Qt::UserRole + 1;
constexpr int DesktopIdRole = Qt::UserRole + 2;
constexpr int LaunchedTimesRole = Qt::UserRole + 3;
constexpr int LastLaunchedTimeRole = Qt::UserRole + 4;
}

class TestFrequentlyUsedProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void filterAcceptsAllWhenNoRecentlyInstalled();
    void filterRejectsItemsInRecentlyInstalled();
    void lessThanByLaunchedTimes();
    void lessThanByLastLaunchedTimeWhenTimesEqual();
    void lessThanFallsToFrequentlyUsedWhenTimesZero();
    void setRecentlyInstalledModelEmitsSignal();
    void setSameRecentlyInstalledModelIsNoop();
    void componentCompleteSortsDescending();
    void classBeginIsNoop();
};

static QStandardItem *makeApp(const QString &name, const QString &desktopId,
                             qint64 launchedTimes, qint64 lastLaunchedTime)
{
    auto item = new QStandardItem;
    item->setData(name, NameRole);
    item->setData(desktopId, DesktopIdRole);
    item->setData(launchedTimes, LaunchedTimesRole);
    item->setData(lastLaunchedTime, LastLaunchedTimeRole);
    return item;
}

static void populate(QStandardItemModel &source)
{
    // high usage, low usage, zero usage
    source.appendRow(makeApp("A", "a.desktop", 100, 1000));
    source.appendRow(makeApp("B", "b.desktop", 50, 500));
    source.appendRow(makeApp("C", "c.desktop", 0, 0));
}

static void configureRoles(FrequentlyUsedProxyModel &proxy)
{
    QVERIFY(proxy.setProperty("desktopIdRole", DesktopIdRole));
    QCOMPARE(proxy.property("desktopIdRole").toInt(), DesktopIdRole);
    QVERIFY(proxy.setProperty("launchedTimesRole", LaunchedTimesRole));
    QCOMPARE(proxy.property("launchedTimesRole").toInt(), LaunchedTimesRole);
    QVERIFY(proxy.setProperty("lastLaunchedTimeRole", LastLaunchedTimeRole));
    QCOMPARE(proxy.property("lastLaunchedTimeRole").toInt(), LastLaunchedTimeRole);
}

void TestFrequentlyUsedProxyModel::filterAcceptsAllWhenNoRecentlyInstalled()
{
    QStandardItemModel source;
    populate(source);

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);

    // no recentlyInstalledModel set -> all rows accepted
    QCOMPARE(proxy.rowCount(), 3);
}

void TestFrequentlyUsedProxyModel::filterRejectsItemsInRecentlyInstalled()
{
    QStandardItemModel source;
    populate(source);

    // recentlyInstalledModel contains "b.desktop"
    QStandardItemModel recentModel;
    recentModel.setItemRoleNames({{ DesktopIdRole, QByteArrayLiteral("desktopId") }});
    auto recentItem = new QStandardItem;
    recentItem->setData(QStringLiteral("b.desktop"), DesktopIdRole);
    recentModel.appendRow(recentItem);

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);
    proxy.setRecentlyInstalledModel(&recentModel);

    // "b.desktop" is in recentlyInstalled -> filtered out
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), DesktopIdRole).toString(), QStringLiteral("a.desktop"));
    QCOMPARE(proxy.data(proxy.index(1, 0), DesktopIdRole).toString(), QStringLiteral("c.desktop"));
}

void TestFrequentlyUsedProxyModel::lessThanByLaunchedTimes()
{
    QStandardItemModel source;
    populate(source);

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::DescendingOrder);

    // descending by launched times: A(100), B(50), C(0)
    QCOMPARE(proxy.data(proxy.index(0, 0), DesktopIdRole).toString(), QStringLiteral("a.desktop"));
    QCOMPARE(proxy.data(proxy.index(1, 0), DesktopIdRole).toString(), QStringLiteral("b.desktop"));
    QCOMPARE(proxy.data(proxy.index(2, 0), DesktopIdRole).toString(), QStringLiteral("c.desktop"));
}

void TestFrequentlyUsedProxyModel::lessThanByLastLaunchedTimeWhenTimesEqual()
{
    QStandardItemModel source;
    // two apps with same launched times but different lastLaunchedTime
    source.appendRow(makeApp("X", "x.desktop", 50, 200));
    source.appendRow(makeApp("Y", "y.desktop", 50, 100));

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::DescendingOrder);

    // same launchedTimes(50) -> compare lastLaunchedTime: X(200) > Y(100)
    QCOMPARE(proxy.data(proxy.index(0, 0), DesktopIdRole).toString(), QStringLiteral("x.desktop"));
    QCOMPARE(proxy.data(proxy.index(1, 0), DesktopIdRole).toString(), QStringLiteral("y.desktop"));
}

void TestFrequentlyUsedProxyModel::lessThanFallsToFrequentlyUsedWhenTimesZero()
{
    QStandardItemModel source;
    // two apps with zero launched times -> falls to lessThenByFrequentlyUsed
    source.appendRow(makeApp("P", "p.desktop", 0, 0));
    source.appendRow(makeApp("Q", "q.desktop", 0, 0));

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::DescendingOrder);

    // both have 0 launched times and 0 lastLaunchedTime -> falls to frequentlyUsedAppIdList
    // (from DConfig, likely empty -> both indexOf return -1 -> equal -> stable source order)
    QCOMPARE(proxy.rowCount(), 2);
    // stable source order: P (row 0) before Q (row 1)
    QCOMPARE(proxy.data(proxy.index(0, 0), DesktopIdRole).toString(), QStringLiteral("p.desktop"));
    QCOMPARE(proxy.data(proxy.index(1, 0), DesktopIdRole).toString(), QStringLiteral("q.desktop"));
}

void TestFrequentlyUsedProxyModel::setRecentlyInstalledModelEmitsSignal()
{
    QStandardItemModel source;
    populate(source);

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);

    QStandardItemModel recentModel;
    QSignalSpy spy(&proxy, &FrequentlyUsedProxyModel::recentlyInstalledModelChanged);
    proxy.setRecentlyInstalledModel(&recentModel);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(proxy.recentlyInstalledModel(), &recentModel);
}

void TestFrequentlyUsedProxyModel::setSameRecentlyInstalledModelIsNoop()
{
    QStandardItemModel source;
    populate(source);

    QStandardItemModel recentModel;

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);
    proxy.setRecentlyInstalledModel(&recentModel);

    QSignalSpy spy(&proxy, &FrequentlyUsedProxyModel::recentlyInstalledModelChanged);
    proxy.setRecentlyInstalledModel(&recentModel); // same model
    QCOMPARE(spy.count(), 0);
}

void TestFrequentlyUsedProxyModel::componentCompleteSortsDescending()
{
    QStandardItemModel source;
    populate(source);

    FrequentlyUsedProxyModel proxy;
    configureRoles(proxy);
    proxy.setSourceModel(&source);
    proxy.componentComplete(); // sort(0, Qt::DescendingOrder)

    QCOMPARE(proxy.data(proxy.index(0, 0), DesktopIdRole).toString(), QStringLiteral("a.desktop"));
}

void TestFrequentlyUsedProxyModel::classBeginIsNoop()
{
    FrequentlyUsedProxyModel proxy;
    proxy.classBegin(); // just logs, no crash
}

QTEST_MAIN(TestFrequentlyUsedProxyModel)
#include "frequentlyusedproxymodeltest.moc"
