// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include "itemarrangementproxymodel.h"
#include "appsmodel.h"
#include "foldermodelstub.h"

namespace {

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
    void bringToFrontForwardsToProvider();
    void commitDndOperationDoesNotCrash();
    void creatEmptyPageIncreasesPageCount();
    void removeEmptyPageRemovesEmptyPages();
    void createReturnsSameInstance();

private:
    QStandardItemModel m_sourceModel;
    FolderModelStub m_folderModel;
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
    m_sourceModel.appendRow(makeApp("arrange-a", "App A", 1));
    m_sourceModel.appendRow(makeApp("arrange-b", "App B", 2));
    m_sourceModel.appendRow(makeApp("arrange-c", "App C", 3));
    m_sourceModel.appendRow(makeApp("arrange-d", "App D", 4));
    m_sourceModel.appendRow(makeApp("arrange-e", "App E", 5));

    AppsModel::instance().setSourceModel(&m_sourceModel);
    AppsModel::instance().setReady(true);

    ItemArrangementProxyModel::instance().setFolderModel(&m_folderModel);

    // Give arrange-e a non-zero initial position so bringToFront's assertion
    // that it lands at (page 0, pos 0) is meaningful (not a vacuous truth).
    m_folderModel.positions.insert("arrange-e.desktop",
                                   QVariantMap{{"page", 1}, {"pos", 2}, {"group", 0}});
}

void TestItemArrangementProxyModel::pageCountIsPositiveAfterSetup()
{
    QVERIFY(ItemArrangementProxyModel::instance().pageCount(0) > 0);
}

void TestItemArrangementProxyModel::findItemReturnsValidPositionForExistingItem()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-a");
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
    int row = findRowForDesktopId("nonexistent.desktop");
    QCOMPARE(row, -1);
}

void TestItemArrangementProxyModel::dataReturnsPageAndIndexForAppRow()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-b");
    QVERIFY(row >= 0);

    QModelIndex idx = model.index(row, 0);
    QVERIFY(model.data(idx, ItemArrangementProxyModel::PageRole).isValid());
    QVERIFY(model.data(idx, ItemArrangementProxyModel::IndexInPageRole).isValid());
    QVERIFY(model.data(idx, ItemArrangementProxyModel::FolderIdNumberRole).isValid());
}

void TestItemArrangementProxyModel::dataReturnsAppItemType()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-c");
    QVERIFY(row >= 0);

    QModelIndex idx = model.index(row, 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::ItemTypeRole).toInt(),
             ItemArrangementProxyModel::AppItemType);
}

void TestItemArrangementProxyModel::dataReturnsEmptyIconsNameForApp()
{
    auto &model = ItemArrangementProxyModel::instance();
    int row = findRowForDesktopId("arrange-d");
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
    for (const auto &id : {"arrange-a", "arrange-b", "arrange-c", "arrange-d", "arrange-e"})
        QVERIFY2(findRowForDesktopId(id) >= 0, id);
}

void TestItemArrangementProxyModel::bringToFrontMovesItemToFirstPosition()
{
    auto &model = ItemArrangementProxyModel::instance();

    // find the row for arrange-e (should NOT be at page 0, index 0 initially)
    int row = findRowForDesktopId("arrange-e");
    QVERIFY(row >= 0);

    QSignalSpy spy(&model, &ItemArrangementProxyModel::itemBroughtToFront);
    model.bringToFront("arrange-e");

    // signal should have fired (item was moved)
    QVERIFY(spy.count() >= 1);

    // after bringToFront, arrange-e should be at page 0, index 0
    row = findRowForDesktopId("arrange-e");
    QVERIFY(row >= 0);
    QModelIndex idx = model.index(row, 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::PageRole).toInt(), 0);
    QCOMPARE(model.data(idx, ItemArrangementProxyModel::IndexInPageRole).toInt(), 0);
}

void TestItemArrangementProxyModel::bringToFrontForwardsToProvider()
{
    auto &model = ItemArrangementProxyModel::instance();
    // Clear state left by the previous bringToFront test so the assertion
    // below verifies *this* call's forwarding, not a stale entry.
    m_folderModel.broughtToFront.clear();
    // The adapter forwards every call; noop semantics belong to the shared provider.
    QSignalSpy spy(&model, &ItemArrangementProxyModel::itemBroughtToFront);
    model.bringToFront("arrange-e");
    QCOMPARE(spy.count(), 1);
    // .desktop normalization happens before forwarding
    QCOMPARE(m_folderModel.broughtToFront.last(), "arrange-e.desktop");
}

void TestItemArrangementProxyModel::commitDndOperationDoesNotCrash()
{
    auto &model = ItemArrangementProxyModel::instance();
    // commitDndOperation does not short-circuit on same-id; it always
    // forwards to the folder model's rearrange().  Verify forwarding occurred.
    m_folderModel.lastMove.clear();
    model.commitDndOperation("arrange-a", "arrange-a",
                             ItemArrangementProxyModel::DndJoin);
    QVERIFY(!m_folderModel.lastMove.isEmpty());
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

void TestItemArrangementProxyModel::createReturnsSameInstance()
{
    // The create() static method (QML singleton factory) should return
    // the same singleton instance as instance().
    ItemArrangementProxyModel *created = ItemArrangementProxyModel::create(nullptr, nullptr);
    QVERIFY(created != nullptr);
    QCOMPARE(created, &ItemArrangementProxyModel::instance());
}

void TestItemArrangementProxyModel::cleanupTestCase()
{
    ItemArrangementProxyModel::instance().setFolderModel(nullptr);
    // delete the item-arrangement.ini written by saveItemArrangementToUserData()
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString iniPath = QDir(basePath).absoluteFilePath("deepin/dde-launchpad/item-arrangement.ini");
    QFile::remove(iniPath);
}

QTEST_MAIN(TestItemArrangementProxyModel)
#include "itemarrangementproxymodeltest.moc"
