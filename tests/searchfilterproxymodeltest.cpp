// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QDebug>

#include "searchfilterproxymodel.h"
#include "appsmodel.h"
#include "categorizedsortproxymodel.h"

namespace {

enum SourceRoles {
    SourceDesktopIdRole = Qt::UserRole + 1,
    SourceNameRole,
    SourceIconNameRole,
    SourceNoDisplayRole,
    SourceDDECategoryRole,
    SourceInstalledTimeRole,
    SourceLastLaunchedTimeRole,
    SourceLaunchedTimesRole,
    SourceAutoStartRole,
    SourceCategoriesRole,
    SourceVendorRole,
    SourceGenericNameRole,
};
}

class TestSearchFilterProxyModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testBasicSearch();
    void testChineseSearch();
    void testPinyinSearch();
    void testJianpinSearch();
    void testSorting();
    void testSpecialCharacters();
    void testSharedModelAdapter();
    void testCategorySortSignalOrder();
    void testCategorizedSortAlphabetarySections();
    void testCategorizedSortDDECategorySections();
    void testCategorizedSortRoleNameAndCategoryType();

private:
    void setupTestData();
    QStandardItem *createTestAppItem(const QString &desktopId,
                                     const QString &name,
                                     const QString &displayName,
                                     const QString &genericName = QString(),
                                     const QString &vendor = QString(),
                                     int launchedTimes = 0);

    QStandardItemModel m_sourceModel;
};

void TestSearchFilterProxyModel::initTestCase()
{
    // 初始化测试环境
    setupTestData();
}

void TestSearchFilterProxyModel::cleanupTestCase()
{
    // 清理测试环境
}

void TestSearchFilterProxyModel::setupTestData()
{
    m_sourceModel.clear();
    m_sourceModel.setItemRoleNames({
        { SourceDesktopIdRole, QByteArrayLiteral("desktopId") },
        { SourceNameRole, QByteArrayLiteral("name") },
        { SourceIconNameRole, QByteArrayLiteral("iconName") },
        { SourceNoDisplayRole, QByteArrayLiteral("noDisplay") },
        { SourceDDECategoryRole, QByteArrayLiteral("ddeCategory") },
        { SourceInstalledTimeRole, QByteArrayLiteral("installedTime") },
        { SourceLastLaunchedTimeRole, QByteArrayLiteral("lastLaunchedTime") },
        { SourceLaunchedTimesRole, QByteArrayLiteral("launchedTimes") },
        { SourceAutoStartRole, QByteArrayLiteral("autoStart") },
        { SourceCategoriesRole, QByteArrayLiteral("categories") },
        { SourceVendorRole, QByteArrayLiteral("vendor") },
        { SourceGenericNameRole, QByteArrayLiteral("genericName") },
    });
    
    // 创建测试数据
    QList<QStandardItem *> testItems;
    
    // 英文应用
    testItems.append(createTestAppItem("org.deepin.calculator", "Calculator", "Calculator", "Calculator", "deepin", 10));
    testItems.append(createTestAppItem("org.deepin.editor", "Text Editor", "Text Editor", "Editor", "deepin", 5));
    testItems.append(createTestAppItem("org.deepin.browser", "Web Browser", "Web Browser", "Browser", "deepin", 20));
    testItems.append(createTestAppItem("org.libreoffice.writer", "LibreOffice Writer", "LibreOffice Writer", "", "libreoffice", 3));
    testItems.append(createTestAppItem("org.gimp.GIMP", "GIMP", "GNU Image Manipulation Program", "", "gimp", 8));
    testItems.append(createTestAppItem("com.visualstudio.code", "Visual Studio Code", "Visual Studio Code", "", "microsoft", 15));
    
    // 中文应用
    testItems.append(createTestAppItem("org.deepin.music", "音乐", "音乐播放器", "音乐", "deepin", 12));
    testItems.append(createTestAppItem("org.deepin.reader", "阅读器", "文档阅读器", "阅读", "deepin", 7));
    testItems.append(createTestAppItem("org.deepin.calendar", "日历", "日历", "日历", "deepin", 4));
    testItems.append(createTestAppItem("org.deepin.defender", "安全中心", "安全中心", "安全中心", "deepin", 4));
    
    // 包含特殊符号的应用
    testItems.append(createTestAppItem("org.special.app1", "App@Name", "App@Name", "", "special", 1));
    testItems.append(createTestAppItem("org.special.app2", "App-Name", "App-Name", "", "special", 2));
    testItems.append(createTestAppItem("org.special.app3", "App_Name", "App_Name", "", "special", 3));
    testItems.append(createTestAppItem("org.special.app4", "App+Name", "App+Name", "", "special", 4));
    testItems.append(createTestAppItem("org.special.app5", "App&Name", "App&Name", "", "special", 5));
    testItems.append(createTestAppItem("org.special.app6", "App(Name)", "App(Name)", "", "special", 6));
    testItems.append(createTestAppItem("org.special.app7", "App[Name]", "App[Name]", "", "special", 7));
    testItems.append(createTestAppItem("org.special.app8", "App{Name}", "App{Name}", "", "special", 8));
    testItems.append(createTestAppItem("org.special.app9", "App!Name", "App!Name", "", "special", 9));
    testItems.append(createTestAppItem("org.special.app10", "App#Name", "App#Name", "", "special", 10));
    testItems.append(createTestAppItem("org.special.app11", "App$Name", "App$Name", "", "special", 11));
    testItems.append(createTestAppItem("org.special.app12", "App%Name", "App%Name", "", "special", 12));
    testItems.append(createTestAppItem("org.special.app13", "App^Name", "App^Name", "", "special", 13));
    testItems.append(createTestAppItem("org.special.app14", "App*Name", "App*Name", "", "special", 14));
    
    // 添加到模型
    for (auto item : std::as_const(testItems))
        m_sourceModel.appendRow(item);
    AppsModel::instance().setSourceModel(&m_sourceModel);
    AppsModel::instance().setReady(true);
}

QStandardItem *TestSearchFilterProxyModel::createTestAppItem(const QString &desktopId,
                                                             const QString &name,
                                                             const QString &displayName,
                                                             const QString &genericName,
                                                             const QString &vendor,
                                                             int launchedTimes)
{
    auto item = new QStandardItem;
    item->setData(desktopId, SourceDesktopIdRole);
    item->setData(displayName.isEmpty() ? name : displayName, SourceNameRole);
    // Empty icon name so AppMgr::waitForIcon() resolves it immediately and does
    // not defer the row (a non-empty unresolvable icon would make the noDisplay
    // unhide in testSharedModelAdapter() delayed, failing the rowCount check).
    item->setData(QString(), SourceIconNameRole);
    item->setData(genericName, SourceGenericNameRole);
    item->setData(vendor, SourceVendorRole);
    item->setData(launchedTimes, SourceLaunchedTimesRole);
    item->setData(launchedTimes % 11, SourceDDECategoryRole);
    return item;
}

void TestSearchFilterProxyModel::testBasicSearch()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试空搜索
    model.setFilterRegularExpression(QRegularExpression(""));
    int totalItems = AppsModel::instance().rowCount();
    QCOMPARE(model.rowCount(), totalItems);
    
    // 测试基本搜索 - 完全匹配
    model.setFilterRegularExpression(QRegularExpression("Calculator"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.calculator.desktop");
    
    // 测试基本搜索 - 部分匹配
    model.setFilterRegularExpression(QRegularExpression("Text"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.editor.desktop");
    
    // 测试基本搜索 - 不区分大小写
    model.setFilterRegularExpression(QRegularExpression("calculator"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.calculator.desktop");
    
    // 测试基本搜索 - 多个结果
    model.setFilterRegularExpression(QRegularExpression("e"));
    QVERIFY(model.rowCount() > 1);
}

void TestSearchFilterProxyModel::testChineseSearch()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试中文完全匹配
    model.setFilterRegularExpression(QRegularExpression("音乐"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.music.desktop");
    
    // 测试中文部分匹配
    model.setFilterRegularExpression(QRegularExpression("阅读"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.reader.desktop");
}

void TestSearchFilterProxyModel::testPinyinSearch()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试拼音搜索
    model.setFilterRegularExpression(QRegularExpression("yinyue"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.music.desktop");
    
    // 测试拼音部分匹配
    model.setFilterRegularExpression(QRegularExpression("yuedu"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.reader.desktop");
    
    // 测试拼音首字母匹配
    model.setFilterRegularExpression(QRegularExpression("rl"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.calendar.desktop");
    model.setFilterRegularExpression(QRegularExpression("aqzx"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.defender.desktop");
}

void TestSearchFilterProxyModel::testJianpinSearch()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试简拼搜索
    model.setFilterRegularExpression(QRegularExpression("yy"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.music.desktop");
    
    // 测试简拼部分匹配
    model.setFilterRegularExpression(QRegularExpression("yd"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.reader.desktop");
}

void TestSearchFilterProxyModel::testSorting()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试排序 - 根据匹配权重和启动次数
    model.setFilterRegularExpression(QRegularExpression("e"));
    int resultCount = model.rowCount();
    QVERIFY(resultCount > 2);
    
    // 验证排序结果
    // 检查前几个结果是否符合预期的排序规则
    bool foundBrowser = false;
    bool foundEditor = false;
    
    int checkCount = qMin(3, resultCount);
    
    for (int i = 0; i < checkCount; ++i) {
        QString desktopId = model.data(model.index(i, 0), AppsModel::DesktopIdRole).toString();
        
        if (desktopId == "org.deepin.browser.desktop") {
            foundBrowser = true;
        } else if (desktopId == "org.deepin.editor.desktop") {
            foundEditor = true;
        }
    }
    
    // 由于浏览器的启动次数更高，它应该排在编辑器前面
    QVERIFY(foundBrowser);
}

void TestSearchFilterProxyModel::testSpecialCharacters()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试普通字符搜索 - 确保基本功能正常
    model.setFilterRegularExpression(QRegularExpression("App"));
    int appCount = model.rowCount();
    QVERIFY(appCount >= 14);
    
    // 测试特殊字符搜索
    model.setFilterRegularExpression(QRegularExpression("App-"));
    int hyphenCount = model.rowCount();
    QVERIFY(hyphenCount > 0);
    
    model.setFilterRegularExpression(QRegularExpression("App_"));
    int underscoreCount = model.rowCount();
    QVERIFY(underscoreCount > 0);
    
    // 验证特殊应用存在
    bool foundSpecialApp = false;
    
    model.setFilterRegularExpression(QRegularExpression("App"));
    int totalAppCount = model.rowCount();
    QVERIFY(totalAppCount > 0);
    
    for (int i = 0; i < totalAppCount; ++i) {
        QString desktopId = model.data(model.index(i, 0), AppsModel::DesktopIdRole).toString();
        
        if (desktopId.startsWith("org.special.app")) {
            foundSpecialApp = true;
            break;
        }
    }
    
    QVERIFY(foundSpecialApp);
}

void TestSearchFilterProxyModel::testSharedModelAdapter()
{
    auto &appsModel = AppsModel::instance();
    const int initialRowCount = appsModel.rowCount();
    QSignalSpy resetSpy(&appsModel, &QAbstractItemModel::modelReset);
    QSignalSpy rowsInsertedSpy(&appsModel, &QAbstractItemModel::rowsInserted);
    QSignalSpy rowsRemovedSpy(&appsModel, &QAbstractItemModel::rowsRemoved);

    auto hiddenItem = createTestAppItem("org.test.hidden", "Hidden", "Hidden");
    hiddenItem->setData(true, SourceNoDisplayRole);
    m_sourceModel.appendRow(hiddenItem);
    QCOMPARE(appsModel.rowCount(), initialRowCount);
    QCOMPARE(rowsInsertedSpy.count(), 0);

    hiddenItem->setData(false, SourceNoDisplayRole);
    QCOMPARE(appsModel.rowCount(), initialRowCount + 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(appsModel.indexFromDesktopId("org.test.hidden.desktop").data(AppsModel::DesktopIdRole).toString(),
             QStringLiteral("org.test.hidden.desktop"));

    auto suffixedItem = createTestAppItem("org.test.suffixed.desktop", "Suffixed", "Suffixed");
    m_sourceModel.appendRow(suffixedItem);
    QCOMPARE(rowsInsertedSpy.count(), 2);
    const QModelIndex suffixedIndex = appsModel.indexFromDesktopId("org.test.suffixed");
    QCOMPARE(suffixedIndex.data(AppsModel::DesktopIdRole).toString(),
             QStringLiteral("org.test.suffixed.desktop"));

    QSignalSpy dataChangedSpy(&appsModel, &QAbstractItemModel::dataChanged);
    suffixedItem->setData(QStringLiteral("Renamed"), SourceNameRole);
    QCOMPARE(appsModel.indexFromDesktopId("org.test.suffixed.desktop").data(Qt::DisplayRole).toString(),
             QStringLiteral("Renamed"));
    QVERIFY(!dataChangedSpy.isEmpty());
    const auto changedRoles = dataChangedSpy.constLast().at(2).value<QList<int>>();
    QVERIFY(changedRoles.contains(Qt::DisplayRole));
    QVERIFY(changedRoles.contains(AppsModel::NameRole));
    QVERIFY(changedRoles.contains(AppsModel::TransliteratedRole));

    m_sourceModel.removeRow(suffixedItem->row());
    QCOMPARE(rowsRemovedSpy.count(), 1);
    m_sourceModel.removeRow(hiddenItem->row());
    QCOMPARE(rowsRemovedSpy.count(), 2);
    QCOMPARE(appsModel.rowCount(), initialRowCount);
    QCOMPARE(resetSpy.count(), 0);
}

void TestSearchFilterProxyModel::testCategorySortSignalOrder()
{
    auto &model = CategorizedSortProxyModel::instance();
    model.setCategoryType(CategorizedSortProxyModel::DDECategory);

    QStringList signalOrder;
    QObject connectionsGuard;
    connect(&model, &CategorizedSortProxyModel::sectionRoleNameChanged, &connectionsGuard,
            [&signalOrder] { signalOrder << QStringLiteral("section"); });
    connect(&model, &QAbstractItemModel::layoutAboutToBeChanged, &connectionsGuard,
            [&signalOrder] { signalOrder << QStringLiteral("layoutAboutToChange"); });
    connect(&model, &QAbstractItemModel::layoutChanged, &connectionsGuard,
            [&signalOrder] { signalOrder << QStringLiteral("layoutChanged"); });
    connect(&model, &CategorizedSortProxyModel::categoryTypeChanged, &connectionsGuard,
            [&signalOrder] { signalOrder << QStringLiteral("category"); });

    model.setCategoryType(CategorizedSortProxyModel::Alphabetary);

    QCOMPARE(model.sectionRoleName(), QStringLiteral("transliterated"));
    QCOMPARE(signalOrder, QStringList({ QStringLiteral("section"),
                                        QStringLiteral("layoutAboutToChange"),
                                        QStringLiteral("layoutChanged"),
                                        QStringLiteral("category") }));
}

void TestSearchFilterProxyModel::testCategorizedSortAlphabetarySections()
{
    // Exercise alphabetarySections(): collect uppercased first chars of transliterated names.
    auto &model = CategorizedSortProxyModel::instance();
    model.setCategoryType(CategorizedSortProxyModel::Alphabetary);

    const auto sections = model.alphabetarySections();
    // The test data contains English apps (Calculator, Editor, Browser, etc.)
    // and Chinese apps (音乐, etc.). Each transliterated name starts with a
    // letter; the set should be non-empty and sorted (with &/# at front if present).
    QVERIFY(!sections.isEmpty());
    // Verify the returned list is sorted by the custom comparator (& < # < letters)
    for (int i = 1; i < sections.size(); ++i) {
        const QString &prev = sections[i - 1];
        const QString &curr = sections[i];
        // customLessThan: & < #, then normal <
        if (prev == "&" && curr == "#")
            continue; // & before # is valid
        QVERIFY(prev != "#" || curr != "&"); // # before & would be wrong order
        QVERIFY(curr >= prev || (prev == "&" && curr == "#"));
    }
}

void TestSearchFilterProxyModel::testCategorizedSortDDECategorySections()
{
    // Exercise DDECategorySections(): collect distinct DDECategory values.
    auto &model = CategorizedSortProxyModel::instance();
    model.setCategoryType(CategorizedSortProxyModel::DDECategory);

    const auto sections = model.DDECategorySections();
    // The test data has items with DDECategoryRole set (via SourceDDECategoryRole % 11)
    // so the section list should be non-empty and sorted ascending.
    QVERIFY(!sections.isEmpty());
    // Verify sorted ascending
    for (int i = 1; i < sections.size(); ++i)
        QVERIFY(sections[i] >= sections[i - 1]);
}

void TestSearchFilterProxyModel::testCategorizedSortRoleNameAndCategoryType()
{
    // Exercise sortRoleName() and categoryType() getter — all 3 branches.
    auto &model = CategorizedSortProxyModel::instance();

    // Set to DDECategory and verify getter returns DDECategory
    model.setCategoryType(CategorizedSortProxyModel::DDECategory);
    QCOMPARE(int(model.categoryType()), int(CategorizedSortProxyModel::DDECategory));
    // sortRoleName should return the name of the DDECategoryRole
    QCOMPARE(model.sortRoleName(), QStringLiteral("category"));

    // Set to Alphabetary and verify getter returns Alphabetary
    model.setCategoryType(CategorizedSortProxyModel::Alphabetary);
    QCOMPARE(int(model.categoryType()), int(CategorizedSortProxyModel::Alphabetary));
    QCOMPARE(model.sortRoleName(), QStringLiteral("transliterated"));

    // FreeCategory branch: when categoryType is FreeCategory, isFreeSort=true
    model.setCategoryType(CategorizedSortProxyModel::FreeCategory);
    QCOMPARE(int(model.categoryType()), int(CategorizedSortProxyModel::FreeCategory));
}

QTEST_MAIN(TestSearchFilterProxyModel)
#include "searchfilterproxymodeltest.moc"
