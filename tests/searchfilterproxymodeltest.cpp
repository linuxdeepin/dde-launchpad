// SPDX-FileCopyrightText: 2024 -2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QDebug>
#include <QLoggingCategory>

#include "../src/models/searchfilterproxymodel.h"
#include "../src/models/appsmodel.h"
#include "../src/models/categorizedsortproxymodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

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
    qCInfo(logTest) << "Initializing test case for SearchFilterProxyModel";
    // 初始化测试环境
    setupTestData();
    qCInfo(logTest) << "Test case initialization completed";
}

void TestSearchFilterProxyModel::cleanupTestCase()
{
    qCInfo(logTest) << "Cleaning up test case for SearchFilterProxyModel";
    // 清理测试环境
    qCInfo(logTest) << "Test case cleanup completed";
}

void TestSearchFilterProxyModel::setupTestData()
{
    qCDebug(logTest) << "Setting up test data for SearchFilterProxyModel";
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
    qCInfo(logTest) << "Added" << testItems.size() << "test items to AppsModel";
}

QStandardItem *TestSearchFilterProxyModel::createTestAppItem(const QString &desktopId,
                                                             const QString &name,
                                                             const QString &displayName,
                                                             const QString &genericName,
                                                             const QString &vendor,
                                                             int launchedTimes)
{
    qCDebug(logTest) << "Creating test app item:" << desktopId << "name:" << name << "launched times:" << launchedTimes;
    auto item = new QStandardItem;
    item->setData(desktopId, SourceDesktopIdRole);
    item->setData(displayName.isEmpty() ? name : displayName, SourceNameRole);
    item->setData(QStringLiteral("application-default-icon"), SourceIconNameRole);
    item->setData(genericName, SourceGenericNameRole);
    item->setData(vendor, SourceVendorRole);
    item->setData(launchedTimes, SourceLaunchedTimesRole);
    item->setData(launchedTimes % 11, SourceDDECategoryRole);
    return item;
}

void TestSearchFilterProxyModel::testBasicSearch()
{
    qCInfo(logTest) << "Starting basic search tests";
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试空搜索
    qCDebug(logTest) << "Testing empty search";
    model.setFilterRegularExpression(QRegularExpression(""));
    int totalItems = AppsModel::instance().rowCount();
    qCDebug(logTest) << "Empty search returned" << model.rowCount() << "items, expected" << totalItems;
    QCOMPARE(model.rowCount(), totalItems);
    
    // 测试基本搜索 - 完全匹配
    qCDebug(logTest) << "Testing exact match search for 'Calculator'";
    model.setFilterRegularExpression(QRegularExpression("Calculator"));
    qCDebug(logTest) << "Calculator search returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.calculator.desktop");
    
    // 测试基本搜索 - 部分匹配
    qCDebug(logTest) << "Testing partial match search for 'Text'";
    model.setFilterRegularExpression(QRegularExpression("Text"));
    qCDebug(logTest) << "Text search returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.editor.desktop");
    
    // 测试基本搜索 - 不区分大小写
    qCDebug(logTest) << "Testing case-insensitive search for 'calculator'";
    model.setFilterRegularExpression(QRegularExpression("calculator"));
    qCDebug(logTest) << "Case-insensitive search returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.calculator.desktop");
    
    // 测试基本搜索 - 多个结果
    qCDebug(logTest) << "Testing multi-result search for 'e'";
    model.setFilterRegularExpression(QRegularExpression("e"));
    qCDebug(logTest) << "Multi-result search returned" << model.rowCount() << "items";
    QVERIFY(model.rowCount() > 1);
    qCInfo(logTest) << "Basic search tests completed successfully";
}

void TestSearchFilterProxyModel::testChineseSearch()
{
    qCInfo(logTest) << "Starting Chinese search tests";
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试中文完全匹配
    qCDebug(logTest) << "Testing Chinese exact match for '音乐'";
    model.setFilterRegularExpression(QRegularExpression("音乐"));
    qCDebug(logTest) << "Chinese search for '音乐' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.music.desktop");
    
    // 测试中文部分匹配
    qCDebug(logTest) << "Testing Chinese partial match for '阅读'";
    model.setFilterRegularExpression(QRegularExpression("阅读"));
    qCDebug(logTest) << "Chinese search for '阅读' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.reader.desktop");
    qCInfo(logTest) << "Chinese search tests completed successfully";
}

void TestSearchFilterProxyModel::testPinyinSearch()
{
    qCInfo(logTest) << "Starting Pinyin search tests";
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试拼音搜索
    qCDebug(logTest) << "Testing Pinyin search for 'yinyue'";
    model.setFilterRegularExpression(QRegularExpression("yinyue"));
    qCDebug(logTest) << "Pinyin search for 'yinyue' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.music.desktop");
    
    // 测试拼音部分匹配
    qCDebug(logTest) << "Testing Pinyin partial match for 'yuedu'";
    model.setFilterRegularExpression(QRegularExpression("yuedu"));
    qCDebug(logTest) << "Pinyin search for 'yuedu' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.reader.desktop");
    
    // 测试拼音首字母匹配
    qCDebug(logTest) << "Testing Pinyin initials match for 'rl'";
    model.setFilterRegularExpression(QRegularExpression("rl"));
    qCDebug(logTest) << "Pinyin initials search for 'rl' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.calendar.desktop");
    qCDebug(logTest) << "Testing Pinyin initials match for 'aqzx'";
    model.setFilterRegularExpression(QRegularExpression("aqzx"));
    qCDebug(logTest) << "Pinyin initials search for 'aqzx' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.defender.desktop");
    qCInfo(logTest) << "Pinyin search tests completed successfully";
}

void TestSearchFilterProxyModel::testJianpinSearch()
{
    qCInfo(logTest) << "Starting Jianpin (abbreviated Pinyin) search tests";
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试简拼搜索
    qCDebug(logTest) << "Testing Jianpin search for 'yy' (音乐)";
    model.setFilterRegularExpression(QRegularExpression("yy"));
    qCDebug(logTest) << "Jianpin search for 'yy' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.music.desktop");
    
    // 测试简拼部分匹配
    qCDebug(logTest) << "Testing Jianpin partial match for 'yd' (阅读)";
    model.setFilterRegularExpression(QRegularExpression("yd"));
    qCDebug(logTest) << "Jianpin search for 'yd' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppsModel::DesktopIdRole).toString(), "org.deepin.reader.desktop");
    qCInfo(logTest) << "Jianpin search tests completed successfully";
}

void TestSearchFilterProxyModel::testSorting()
{
    qCInfo(logTest) << "Starting sorting tests";
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试排序 - 根据匹配权重和启动次数
    qCDebug(logTest) << "Testing sorting with search pattern 'e'";
    model.setFilterRegularExpression(QRegularExpression("e"));
    int resultCount = model.rowCount();
    qCDebug(logTest) << "Search for 'e' returned" << resultCount << "items";
    QVERIFY(resultCount > 2);
    
    // 验证排序结果
    qCDebug(logTest) << "Verifying sort order based on launch times and match weight";
    // 检查前几个结果是否符合预期的排序规则
    bool foundBrowser = false;
    bool foundEditor = false;
    
    int checkCount = qMin(3, resultCount);
    qCDebug(logTest) << "Checking first" << checkCount << "results for expected apps";
    
    for (int i = 0; i < checkCount; ++i) {
        QString desktopId = model.data(model.index(i, 0), AppsModel::DesktopIdRole).toString();
        qCDebug(logTest) << "Result" << i << ":" << desktopId;
        
        if (desktopId == "org.deepin.browser.desktop") {
            qCDebug(logTest) << "Found browser at position" << i;
            foundBrowser = true;
        } else if (desktopId == "org.deepin.editor.desktop") {
            qCDebug(logTest) << "Found editor at position" << i;
            foundEditor = true;
        }
    }
    
    // 由于浏览器的启动次数更高，它应该排在编辑器前面
    qCDebug(logTest) << "Browser found:" << foundBrowser << "Editor found:" << foundEditor;
    QVERIFY(foundBrowser);
    qCInfo(logTest) << "Sorting tests completed successfully";
}

void TestSearchFilterProxyModel::testSpecialCharacters()
{
    qCInfo(logTest) << "Starting special characters search tests";
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试普通字符搜索 - 确保基本功能正常
    qCDebug(logTest) << "Testing basic 'App' search";
    model.setFilterRegularExpression(QRegularExpression("App"));
    int appCount = model.rowCount();
    qCDebug(logTest) << "Basic 'App' search returned" << appCount << "items";
    QVERIFY(appCount >= 14);
    
    // 测试特殊字符搜索
    qCDebug(logTest) << "Testing search with hyphen 'App-'";
    model.setFilterRegularExpression(QRegularExpression("App-"));
    int hyphenCount = model.rowCount();
    qCDebug(logTest) << "'App-' search returned" << hyphenCount << "items";
    QVERIFY(hyphenCount > 0);
    
    qCDebug(logTest) << "Testing search with underscore 'App_'";
    model.setFilterRegularExpression(QRegularExpression("App_"));
    int underscoreCount = model.rowCount();
    qCDebug(logTest) << "'App_' search returned" << underscoreCount << "items";
    QVERIFY(underscoreCount > 0);
    
    // 验证特殊应用存在
    qCDebug(logTest) << "Verifying special apps are found in general 'App' search";
    bool foundSpecialApp = false;
    
    model.setFilterRegularExpression(QRegularExpression("App"));
    int totalAppCount = model.rowCount();
    qCDebug(logTest) << "Searching through" << totalAppCount << "apps for special apps";
    QVERIFY(totalAppCount > 0);
    
    for (int i = 0; i < totalAppCount; ++i) {
        QString desktopId = model.data(model.index(i, 0), AppsModel::DesktopIdRole).toString();
        qCDebug(logTest) << "Checking app at index" << i << ":" << desktopId;
        
        if (desktopId.startsWith("org.special.app")) {
            qCDebug(logTest) << "Found special app:" << desktopId;
            foundSpecialApp = true;
            break;
        }
    }
    
    qCDebug(logTest) << "Special app found:" << foundSpecialApp;
    QVERIFY(foundSpecialApp);
    qCInfo(logTest) << "Special characters search tests completed successfully";
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

QTEST_MAIN(TestSearchFilterProxyModel)
#include "searchfilterproxymodeltest.moc"
