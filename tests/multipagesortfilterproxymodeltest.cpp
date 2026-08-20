// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLoggingCategory>

#include "../src/models/multipagesortfilterproxymodel.h"
#include "../src/models/itemarrangementproxymodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

constexpr int NameRole = Qt::UserRole + 10;
}

class TestMultipageSortFilterProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void filtersByFolderAndPage();
    void noMatchYieldsEmpty();
    void filterOnlyModeChangesSorting();
    void filterOnlyModeToggleDoesNotAutoResort();
};

// build the source in the order: Z, Y, X, W (so the proxy actually has to sort)
static QStandardItem *makeArrangedItem(const QString &name, int folderId, int page, int indexInPage)
{
    auto item = new QStandardItem;
    item->setData(name, NameRole);
    item->setData(folderId, ItemArrangementProxyModel::FolderIdNumberRole);
    item->setData(page, ItemArrangementProxyModel::PageRole);
    item->setData(indexInPage, ItemArrangementProxyModel::IndexInPageRole);
    return item;
}

static void populateSource(QStandardItemModel &source)
{
    source.appendRow(makeArrangedItem(QStringLiteral("Z"), 0, 1, 0)); // source row 0
    source.appendRow(makeArrangedItem(QStringLiteral("Y"), 0, 0, 1)); // source row 1
    source.appendRow(makeArrangedItem(QStringLiteral("X"), 0, 0, 0)); // source row 2
    source.appendRow(makeArrangedItem(QStringLiteral("W"), 1, 0, 0)); // source row 3
}

void TestMultipageSortFilterProxyModel::filtersByFolderAndPage()
{
    qCInfo(logTest) << "Filter by folderId and pageId, sorted by page then index-in-page";
    QStandardItemModel source;
    populateSource(source);

    MultipageSortFilterProxyModel proxy;
    // folderId / pageId members are not initialized by the constructor, set them up front.
    proxy.setProperty("folderId", 0);
    QCOMPARE(proxy.property("folderId").toInt(), 0);
    proxy.setProperty("pageId", -1);
    QCOMPARE(proxy.property("pageId").toInt(), -1);
    proxy.setProperty("filterOnlyMode", false);
    QCOMPARE(proxy.property("filterOnlyMode").toBool(), false);
    proxy.setModel(&source);

    // folder 0, all pages -> X(0,0,0), Y(0,0,1), Z(0,1,0)
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("Y"));
    QCOMPARE(proxy.data(proxy.index(2, 0), NameRole).toString(), QStringLiteral("Z"));

    // restrict to page 0 -> X, Y
    proxy.setProperty("pageId", 0);
    QCOMPARE(proxy.property("pageId").toInt(), 0);
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("Y"));

    // restrict to page 1 -> Z
    proxy.setProperty("pageId", 1);
    QCOMPARE(proxy.property("pageId").toInt(), 1);
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("Z"));

    // switch to folder 1, all pages -> W
    proxy.setProperty("folderId", 1);
    QCOMPARE(proxy.property("folderId").toInt(), 1);
    proxy.setProperty("pageId", -1);
    QCOMPARE(proxy.property("pageId").toInt(), -1);
    QCOMPARE(proxy.rowCount(), 1);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("W"));
}

void TestMultipageSortFilterProxyModel::noMatchYieldsEmpty()
{
    qCInfo(logTest) << "A folderId matching no source row should yield an empty proxy";
    QStandardItemModel source;
    populateSource(source);

    MultipageSortFilterProxyModel proxy;
    proxy.setProperty("folderId", 999);
    QCOMPARE(proxy.property("folderId").toInt(), 999);
    proxy.setProperty("pageId", -1);
    QCOMPARE(proxy.property("pageId").toInt(), -1);
    proxy.setModel(&source);
    QCOMPARE(proxy.rowCount(), 0);
}

void TestMultipageSortFilterProxyModel::filterOnlyModeChangesSorting()
{
    qCInfo(logTest) << "filterOnlyMode=true, after a forced re-sort, should fall back to sortRole-only ordering";
    QStandardItemModel source;
    populateSource(source);

    MultipageSortFilterProxyModel proxy;
    proxy.setProperty("folderId", 0);
    QCOMPARE(proxy.property("folderId").toInt(), 0);
    proxy.setProperty("pageId", -1);
    QCOMPARE(proxy.property("pageId").toInt(), -1);
    proxy.setProperty("filterOnlyMode", false);
    QCOMPARE(proxy.property("filterOnlyMode").toBool(), false);
    proxy.setModel(&source);
    // default ordering (filterOnlyMode=false) by page then index-in-page -> X, Y, Z
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("Y"));
    QCOMPARE(proxy.data(proxy.index(2, 0), NameRole).toString(), QStringLiteral("Z"));

    // Switching filterOnlyMode updates the member (read back to prove it took effect)...
    proxy.setProperty("filterOnlyMode", true);
    QCOMPARE(proxy.property("filterOnlyMode").toBool(), true);
    // ...but onFilterOnlyModeChanged is not wired to any re-sort/invalidate in the source,
    // so the order does NOT change yet (see filterOnlyModeToggleDoesNotAutoResort).
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("Y"));
    QCOMPARE(proxy.data(proxy.index(2, 0), NameRole).toString(), QStringLiteral("Z"));

    // Force a re-evaluation: invalidate() re-runs the sort with the now-active
    // filterOnlyMode==true lessThan (QSortFilterProxyModel::lessThan by sortRole,
    // which is FolderIdNumberRole == 0 for all rows) -> stable source order Z, Y, X.
    // (sort(0) alone would short-circuit: column/order equal to the sort(0) done in setModel.)
    proxy.invalidate();
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("Z"));
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("Y"));
    QCOMPARE(proxy.data(proxy.index(2, 0), NameRole).toString(), QStringLiteral("X"));
}

void TestMultipageSortFilterProxyModel::filterOnlyModeToggleDoesNotAutoResort()
{
    qCInfo(logTest) << "Toggling filterOnlyMode alone must not re-sort (signal is not wired in source)";
    QStandardItemModel source;
    populateSource(source);

    MultipageSortFilterProxyModel proxy;
    proxy.setProperty("folderId", 0);
    QCOMPARE(proxy.property("folderId").toInt(), 0);
    proxy.setProperty("pageId", -1);
    QCOMPARE(proxy.property("pageId").toInt(), -1);
    proxy.setProperty("filterOnlyMode", false);
    QCOMPARE(proxy.property("filterOnlyMode").toBool(), false);
    proxy.setModel(&source);
    // filterOnlyMode=false ordering: X, Y, Z
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(2, 0), NameRole).toString(), QStringLiteral("Z"));

    // Toggling filterOnlyMode emits onFilterOnlyModeChanged, but the source never
    // connects it to invalidate()/invalidateFilter(). The member is updated (read
    // back below), yet no re-sort/re-filter is triggered, so the visible order is
    // unchanged.
    proxy.setProperty("filterOnlyMode", true);
    QCOMPARE(proxy.property("filterOnlyMode").toBool(), true);
    proxy.sort(0); // short-circuits: same column/order as the sort(0) inside setModel
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("Y"));
    QCOMPARE(proxy.data(proxy.index(2, 0), NameRole).toString(), QStringLiteral("Z"));

    // Even flipping the sort order does NOT yield source order here: with
    // filterOnlyMode==true the sortRole (FolderIdNumberRole) is identical for every
    // row, so the stable sort preserves the *current* proxy mapping (X, Y, Z)
    // rather than re-deriving source order. Only invalidate() re-builds from the
    // source (see filterOnlyModeChangesSorting for that case).
    proxy.sort(0, Qt::DescendingOrder);
    QCOMPARE(proxy.data(proxy.index(0, 0), NameRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(1, 0), NameRole).toString(), QStringLiteral("Y"));
    QCOMPARE(proxy.data(proxy.index(2, 0), NameRole).toString(), QStringLiteral("Z"));
}

QTEST_MAIN(TestMultipageSortFilterProxyModel)
#include "multipagesortfilterproxymodeltest.moc"
