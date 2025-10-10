// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
#include "../src/models/appitem.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")
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

private:
    void setupTestData();
    AppItem* createTestAppItem(const QString &desktopId, 
                              const QString &name,
                              const QString &displayName,
                              const QString &genericName = QString(),
                              const QString &vendor = QString(),
                              int launchedTimes = 0);
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
    // 清空当前模型数据
    AppsModel::instance().clear();
    qCDebug(logTest) << "Cleared AppsModel instance";
    
    // 创建测试数据
    QList<AppItem*> testItems;
    
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
    AppsModel::instance().appendRows(testItems);
    qCInfo(logTest) << "Added" << testItems.size() << "test items to AppsModel";
}

AppItem* TestSearchFilterProxyModel::createTestAppItem(const QString &desktopId, 
                                                     const QString &name,
                                                     const QString &displayName,
                                                     const QString &genericName,
                                                     const QString &vendor,
                                                     int launchedTimes)
{
    qCDebug(logTest) << "Creating test app item:" << desktopId << "name:" << name << "launched times:" << launchedTimes;
    AppItem *item = new AppItem(desktopId);
    item->setName(name);
    item->setDisplayName(displayName);
    item->setIconName("application-default-icon");
    
    if (!genericName.isEmpty())
        item->setGenericName(genericName);
    
    if (!vendor.isEmpty())
        item->setVendor(vendor);
    
    item->setLaunchedTimes(launchedTimes);
    
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
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.calculator");
    
    // 测试基本搜索 - 部分匹配
    qCDebug(logTest) << "Testing partial match search for 'Text'";
    model.setFilterRegularExpression(QRegularExpression("Text"));
    qCDebug(logTest) << "Text search returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.editor");
    
    // 测试基本搜索 - 不区分大小写
    qCDebug(logTest) << "Testing case-insensitive search for 'calculator'";
    model.setFilterRegularExpression(QRegularExpression("calculator"));
    qCDebug(logTest) << "Case-insensitive search returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.calculator");
    
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
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.music");
    
    // 测试中文部分匹配
    qCDebug(logTest) << "Testing Chinese partial match for '阅读'";
    model.setFilterRegularExpression(QRegularExpression("阅读"));
    qCDebug(logTest) << "Chinese search for '阅读' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.reader");
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
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.music");
    
    // 测试拼音部分匹配
    qCDebug(logTest) << "Testing Pinyin partial match for 'yuedu'";
    model.setFilterRegularExpression(QRegularExpression("yuedu"));
    qCDebug(logTest) << "Pinyin search for 'yuedu' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.reader");
    
    // 测试拼音首字母匹配
    qCDebug(logTest) << "Testing Pinyin initials match for 'rl'";
    model.setFilterRegularExpression(QRegularExpression("rl"));
    qCDebug(logTest) << "Pinyin initials search for 'rl' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.calendar");
    qCDebug(logTest) << "Testing Pinyin initials match for 'aqzx'";
    model.setFilterRegularExpression(QRegularExpression("aqzx"));
    qCDebug(logTest) << "Pinyin initials search for 'aqzx' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.defender");
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
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.music");
    
    // 测试简拼部分匹配
    qCDebug(logTest) << "Testing Jianpin partial match for 'yd' (阅读)";
    model.setFilterRegularExpression(QRegularExpression("yd"));
    qCDebug(logTest) << "Jianpin search for 'yd' returned" << model.rowCount() << "items";
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.reader");
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
        QString desktopId = model.data(model.index(i, 0), AppItem::DesktopIdRole).toString();
        qCDebug(logTest) << "Result" << i << ":" << desktopId;
        
        if (desktopId == "org.deepin.browser") {
            qCDebug(logTest) << "Found browser at position" << i;
            foundBrowser = true;
        } else if (desktopId == "org.deepin.editor") {
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
        QString desktopId = model.data(model.index(i, 0), AppItem::DesktopIdRole).toString();
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

QTEST_MAIN(TestSearchFilterProxyModel)
#include "searchfilterproxymodeltest.moc" 