// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLoggingCategory>

#include "../src/models/recentlyinstalledproxymodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

constexpr int NameRole = Qt::UserRole + 1;
constexpr int InstalledTimeRole = Qt::UserRole + 2;
constexpr int LastLaunchedTimeRole = Qt::UserRole + 3;
}

class TestRecentlyInstalledProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void filtersAndSortsByInstalledTime();
    void ascendingOrder();
    void componentCompleteSortsDescending();
};

static QStandardItem *makeApp(const QString &name, qint64 installed, qint64 launched)
{
    auto item = new QStandardItem;
    item->setData(name, NameRole);
    item->setData(installed, InstalledTimeRole);
    item->setData(launched, LastLaunchedTimeRole);
    return item;
}

static void populateSource(QStandardItemModel &source)
{
    source.appendRow(makeApp(QStringLiteral("A"), 100, 0));   // installed, never launched -> kept
    source.appendRow(makeApp(QStringLiteral("B"), 200, 0));   // installed, never launched -> kept
    source.appendRow(makeApp(QStringLiteral("C"), 300, 50));  // already launched -> filtered out
    source.appendRow(makeApp(QStringLiteral("D"), 0, 0));     // not installed -> filtered out
    source.appendRow(makeApp(QStringLiteral("E"), 0, 10));    // launched but not installed -> filtered out
}

// the role members are intentionally not initialized by the constructor,
// so they must be set before any filtering/sorting happens. The readback
// QCOMPAREs guard against a silent drift of the Q_PROPERTY name (a wrong
// name would make property() return an invalid variant -> toInt()==0).
static void configureProxy(RecentlyInstalledProxyModel &proxy, QStandardItemModel &source)
{
    proxy.setProperty("installedTimeRole", InstalledTimeRole);
    QCOMPARE(proxy.property("installedTimeRole").toInt(), InstalledTimeRole);
    proxy.setProperty("lastLaunchedTimeRole", LastLaunchedTimeRole);
    QCOMPARE(proxy.property("lastLaunchedTimeRole").toInt(), LastLaunchedTimeRole);
    proxy.setSourceModel(&source);
}

void TestRecentlyInstalledProxyModel::filtersAndSortsByInstalledTime()
{
    qCInfo(logTest) << "Only never-launched, installed apps should be kept, newest first";
    QStandardItemModel source;
    populateSource(source);

    RecentlyInstalledProxyModel proxy;
    configureProxy(proxy, source);
    proxy.sort(0, Qt::DescendingOrder);

    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("B")); // installed=200
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("A")); // installed=100
}

void TestRecentlyInstalledProxyModel::ascendingOrder()
{
    qCInfo(logTest) << "Ascending sort should order by installed time ascending";
    QStandardItemModel source;
    populateSource(source);

    RecentlyInstalledProxyModel proxy;
    configureProxy(proxy, source);
    proxy.sort(0, Qt::AscendingOrder);

    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("A")); // installed=100
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("B")); // installed=200
}

void TestRecentlyInstalledProxyModel::componentCompleteSortsDescending()
{
    qCInfo(logTest) << "componentComplete() should sort by installed time descending (newest first)";
    QStandardItemModel source;
    populateSource(source);

    RecentlyInstalledProxyModel proxy;
    configureProxy(proxy, source);
    // componentComplete() is what QML calls once the declarative object is ready;
    // it issues sort(0, Qt::DescendingOrder), i.e. newest installed first.
    proxy.componentComplete();

    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("B")); // installed=200
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("A")); // installed=100
}

QTEST_MAIN(TestRecentlyInstalledProxyModel)
#include "recentlyinstalledproxymodeltest.moc"
