// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLoggingCategory>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include "../src/models/itemarrangementproxymodel.h"
#include "../src/models/appsmodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

enum SourceRoles {
    SrcDesktopIdRole = Qt::UserRole + 1,
    SrcNameRole,
    SrcIconNameRole,
    SrcNoDisplayRole,
    SrcDDECategoryRole,
    SrcInstalledTimeRole,
    SrcLastLaunchedTimeRole,
    SrcLaunchedTimesRole,
    SrcAutoStartRole,
    SrcCategoriesRole,
    SrcVendorRole,
    SrcGenericNameRole,
};

QStandardItem *makeApp(const QString &desktopId, const QString &name, int category = 0)
{
    auto item = new QStandardItem;
    item->setData(desktopId, SrcDesktopIdRole);
    item->setData(name, SrcNameRole);
    item->setData(QStringLiteral("application-default-icon"), SrcIconNameRole);
    item->setData(false, SrcNoDisplayRole);
    item->setData(category, SrcDDECategoryRole);
    return item;
}
}

class TestItemArrangementProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void pageCountIsPositiveAfterSetup();
    void findItemReturnsValidPositionForExistingItem();
    void findItemReturnsInvalidForNonexistent();
    void dataReturnsPageAndIndexForAppRow();
    void dataReturnsAppItemType();
    void dataReturnsEmptyIconsNameForApp();
    void roleNamesContainsExtendedRoles();
    void allArrangedItemsContainsAllApps();
    void bringToFrontMovesItemToFirstPosition();
    void bringToFrontOnNonexistentIsNoop();
    void bringToFrontOnFirstItemIsNoop();
    void commitDndOperationWithSameIdIsNoop();
    void creatEmptyPageIncreasesPageCount();
    void removeEmptyPageRemovesEmptyPages();

private:
    QStandardItemModel m_sourceModel;
    int findRowForDesktopId(const QString &desktopId);
};

int TestItemArrangementProxyModel::findRowForDesktopId(const QString &desktopId)
{
    auto &model = ItemArrangementProxyModel::instance();
    for (int i = 0; i < model.rowCount(); ++i) {
        if (model.data(model.index(i, 0), AppsModel::DesktopIdRole).toString() == desktopId)
            return i;
    }
    return -1;
}

void TestItemArrangementProxyModel::initTestCase()
{
    m_sourceModel.setItemRoleNames({
        { SrcDesktopIdRole, QByteArrayLiteral("desktopId") },
        { SrcNameRole, QByteArrayLiteral("name") },
        { SrcIconNameRole, QByteArrayLiteral("iconName") },
        { SrcNoDisplayRole, QByteArrayLiteral("noDisplay") },
        { SrcDDECategoryRole, QByteArrayLiteral("ddeCategory") },
        { SrcInstalledTimeRole, QByteArrayLiteral("installedTime") },
        { SrcLastLaunchedTimeRole, QByteArrayLiteral("lastLaunchedTime") },
        { SrcLaunchedTimesRole, QByteArrayLiteral("launchedTimes") },
        { SrcAutoStartRole, QByteArrayLiteral("autoStart") },
        { SrcCategoriesRole, QByteArrayLiteral("categories") },
        { SrcVendorRole, QByteArrayLiteral("vendor") },
        { SrcGenericNameRole, QByteArrayLiteral("genericName") },
    });
    m_sourceModel.appendRow(makeApp("arrange-a.desktop", "App A", 1));
    m_sourceModel.appendRow(makeApp("arrange-b.desktop", "App B", 2));
    m_sourceModel.appendRow(makeApp("arrange-c.desktop", "App C", 3));
    m_sourceModel.appendRow(makeApp("arrange-d.desktop", "App D", 4));
    m_sourceModel.appendRow(makeApp("arrange-e.desktop", "App E", 5));

    AppsModel::instance().setSourceModel(&m_sourceModel);
    AppsModel::instance().setReady(true);

    // Accessing the singleton triggers construction + onSourceModelChanged,
    // which adds all AppsModel items to topLevel pages.
    ItemArrangementProxyModel::instance();
}

void TestItemArrangementProxyModel::pageCountIsPositiveAfterSetup()
{
    QVERIFY(ItemArrangementProxyModel::instance().pageCount(0) > 0);
}

void TestItemArrangementProxyModel::findItemReturnsValidPositionForExistingItem()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-a.desktop");
    QVERIFY(row >= 0);

    QModelIndex idx = model.index(row, 0);
    int folder = model.data(idx, ItemArrangementProxyModel::FolderIdNumberRole).toInt();
    int page = model.data(idx, ItemArrangementProxyModel::PageRole).toInt();
    int indexInPage = model.data(idx, ItemArrangementProxyModel::IndexInPageRole).toInt();

    // should be in top-level folder (0)
    QCOMPARE(folder, 0);
    QVERIFY(page >= 0);
    QVERIFY(indexInPage >= 0);
}

void TestItemArrangementProxyModel::findItemReturnsInvalidForNonexistent()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("nonexistent.desktop");
    QCOMPARE(row, -1);
}

void TestItemArrangementProxyModel::dataReturnsPageAndIndexForAppRow()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-b.desktop");
    QVERIFY(row >= 0);

    QModelIndex idx = model.index(row, 0);
    QVERIFY(model.data(idx, ItemArrangementProxyModel::PageRole).isValid());
    QVERIFY(model.data(idx, ItemArrangementProxyModel::IndexInPageRole).isValid());
    QVERIFY(model.data(idx, ItemArrangementProxyModel::FolderIdNumberRole).isValid());
}

void TestItemArrangementProxyModel::dataReturnsAppItemType()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-c.desktop");
    QVERIFY(row >= 0);

    QModelIndex idx = model.index(row, 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::ItemTypeRole).toInt(),
             ItemArrangementProxyModel::AppItemType);
}

void TestItemArrangementProxyModel::dataReturnsEmptyIconsNameForApp()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-d.desktop");
    QVERIFY(row >= 0);

    QModelIndex idx = model.index(row, 0);
    QVERIFY(!model.data(idx, ItemArrangementProxyModel::IconsNameRole).isValid());
}

void TestItemArrangementProxyModel::roleNamesContainsExtendedRoles()
{
    auto names = ItemArrangementProxyModel::instance().roleNames();
    // Source roleNames() only inserts IconsNameRole and ItemTypeRole (see
    // itemarrangementproxymodel.cpp roleNames()); PageRole/IndexInPageRole/
    // FolderIdNumberRole are NOT in roleNames() — they are extended roles served
    // via data() but not declared in the role-name hash.
    QVERIFY(names.contains(ItemArrangementProxyModel::IconsNameRole));
    QVERIFY(names.contains(ItemArrangementProxyModel::ItemTypeRole));
    // the three page-arrangement roles are data-only, not in roleNames()
    QVERIFY(!names.contains(ItemArrangementProxyModel::PageRole));
    QVERIFY(!names.contains(ItemArrangementProxyModel::IndexInPageRole));
    QVERIFY(!names.contains(ItemArrangementProxyModel::FolderIdNumberRole));
}

void TestItemArrangementProxyModel::allArrangedItemsContainsAllApps()
{
    // allArrangedItems is private, but itemsPage() is public and returns m_topLevel
    auto all = ItemArrangementProxyModel::instance().itemsPage()->allArrangedItems();
    QVERIFY(all.contains("arrange-a.desktop"));
    QVERIFY(all.contains("arrange-b.desktop"));
    QVERIFY(all.contains("arrange-c.desktop"));
    QVERIFY(all.contains("arrange-d.desktop"));
    QVERIFY(all.contains("arrange-e.desktop"));
}

void TestItemArrangementProxyModel::bringToFrontMovesItemToFirstPosition()
{
    auto &model = ItemArrangementProxyModel::instance();

    // find the row for arrange-e (should NOT be at page 0, index 0 initially)
    int row = findRowForDesktopId("arrange-e.desktop");
    QVERIFY(row >= 0);

    QSignalSpy spy(&model, &ItemArrangementProxyModel::itemBroughtToFront);
    model.bringToFront("arrange-e.desktop");

    // signal should have fired (item was moved)
    QVERIFY(spy.count() >= 1);

    // after bringToFront, arrange-e should be at page 0, index 0
    row = findRowForDesktopId("arrange-e.desktop");
    QVERIFY(row >= 0);
    QModelIndex idx = model.index(row, 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::PageRole).toInt(), 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::IndexInPageRole).toInt(), 0);
}

void TestItemArrangementProxyModel::bringToFrontOnNonexistentIsNoop()
{
    auto &model = ItemArrangementProxyModel::instance();
    QSignalSpy spy(&model, &ItemArrangementProxyModel::itemBroughtToFront);
    model.bringToFront("nonexistent-id.desktop");
    QCOMPARE(spy.count(), 0); // no signal emitted
}

void TestItemArrangementProxyModel::bringToFrontOnFirstItemIsNoop()
{
    auto &model = ItemArrangementProxyModel::instance();

    // self-establish: bring arrange-b to front first so it's at (0, 0)
    int row = findRowForDesktopId("arrange-b.desktop");
    QVERIFY(row >= 0);
    model.bringToFront("arrange-b.desktop");

    // verify it's now at page 0, index 0
    row = findRowForDesktopId("arrange-b.desktop");
    QVERIFY(row >= 0);
    QModelIndex idx = model.index(row, 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::PageRole).toInt(), 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::IndexInPageRole).toInt(), 0);

    // bringToFront on an item already at (0, 0) should be noop
    QSignalSpy spy(&model, &ItemArrangementProxyModel::itemBroughtToFront);
    model.bringToFront("arrange-b.desktop");
    QCOMPARE(spy.count(), 0);
}

void TestItemArrangementProxyModel::commitDndOperationWithSameIdIsNoop()
{
    auto &model = ItemArrangementProxyModel::instance();
    // dragging onto itself should return early
    model.commitDndOperation("arrange-a.desktop", "arrange-a.desktop",
                             ItemArrangementProxyModel::DndJoin);
    // no crash, no assertion failure
}

void TestItemArrangementProxyModel::creatEmptyPageIncreasesPageCount()
{
    auto &model = ItemArrangementProxyModel::instance();
    int before = model.pageCount(0);
    int newPageIndex = model.creatEmptyPage(0);
    QVERIFY(newPageIndex >= 0);
    QCOMPARE(model.pageCount(0), before + 1);

    // cleanup: remove the empty page we just created so it doesn't leak into
    // subsequent tests (removeEmptyPages removes ALL empty pages).
    model.removeEmptyPage();
    QCOMPARE(model.pageCount(0), before);
}

void TestItemArrangementProxyModel::removeEmptyPageRemovesEmptyPages()
{
    auto &model = ItemArrangementProxyModel::instance();
    // self-establish: record the baseline (real pages with apps, no empty pages)
    int baseline = model.pageCount(0);
    // ensure there are no pre-existing empty pages from prior tests
    model.removeEmptyPage();
    QCOMPARE(model.pageCount(0), baseline);

    // create an empty page -> count increases by 1
    model.creatEmptyPage(0);
    QCOMPARE(model.pageCount(0), baseline + 1);

    // remove empty pages -> the page we just created should be gone
    model.removeEmptyPage();
    QCOMPARE(model.pageCount(0), baseline);
}

void TestItemArrangementProxyModel::cleanupTestCase()
{
    // delete the item-arrangement.ini written by saveItemArrangementToUserData()
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString iniPath = QDir(basePath).absoluteFilePath("deepin/dde-launchpad/item-arrangement.ini");
    QFile::remove(iniPath);
}

QTEST_MAIN(TestItemArrangementProxyModel)
#include "itemarrangementproxymodeltest.moc"
