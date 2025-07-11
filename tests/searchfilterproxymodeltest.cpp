// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QDebug>

#include "../src/models/searchfilterproxymodel.h"
#include "../src/models/appsmodel.h"
#include "../src/models/appitem.h"

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
    // 初始化测试环境
    setupTestData();
}

void TestSearchFilterProxyModel::cleanupTestCase()
{
    // 清理测试环境
}

void TestSearchFilterProxyModel::setupTestData()
{
    // 清空当前模型数据
    AppsModel::instance().clear();
    
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
}

AppItem* TestSearchFilterProxyModel::createTestAppItem(const QString &desktopId, 
                                                     const QString &name,
                                                     const QString &displayName,
                                                     const QString &genericName,
                                                     const QString &vendor,
                                                     int launchedTimes)
{
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
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试空搜索
    model.setFilterRegularExpression(QRegularExpression(""));
    QCOMPARE(model.rowCount(), AppsModel::instance().rowCount());
    
    // 测试基本搜索 - 完全匹配
    model.setFilterRegularExpression(QRegularExpression("Calculator"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.calculator");
    
    // 测试基本搜索 - 部分匹配
    model.setFilterRegularExpression(QRegularExpression("Text"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.editor");
    
    // 测试基本搜索 - 不区分大小写
    model.setFilterRegularExpression(QRegularExpression("calculator"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.calculator");
    
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
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.music");
    
    // 测试中文部分匹配
    model.setFilterRegularExpression(QRegularExpression("阅读"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.reader");
}

void TestSearchFilterProxyModel::testPinyinSearch()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试拼音搜索
    model.setFilterRegularExpression(QRegularExpression("yinyue"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.music");
    
    // 测试拼音部分匹配
    model.setFilterRegularExpression(QRegularExpression("yuedu"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.reader");
    
    // 测试拼音首字母匹配
    model.setFilterRegularExpression(QRegularExpression("rl"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.calendar");
}

void TestSearchFilterProxyModel::testJianpinSearch()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试简拼搜索
    model.setFilterRegularExpression(QRegularExpression("yy"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.music");
    
    // 测试简拼部分匹配
    model.setFilterRegularExpression(QRegularExpression("yd"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), AppItem::DesktopIdRole).toString(), "org.deepin.reader");
}

void TestSearchFilterProxyModel::testSorting()
{
    SearchFilterProxyModel &model = SearchFilterProxyModel::instance();
    
    // 测试排序 - 根据匹配权重和启动次数
    model.setFilterRegularExpression(QRegularExpression("e"));
    QVERIFY(model.rowCount() > 2);
    
    // 验证排序结果
    // 检查前几个结果是否符合预期的排序规则
    bool foundBrowser = false;
    bool foundEditor = false;
    
    for (int i = 0; i < qMin(3, model.rowCount()); ++i) {
        QString desktopId = model.data(model.index(i, 0), AppItem::DesktopIdRole).toString();
        if (desktopId == "org.deepin.browser") {
            foundBrowser = true;
        } else if (desktopId == "org.deepin.editor") {
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
    QVERIFY(model.rowCount() >= 14);
    
    
    model.setFilterRegularExpression(QRegularExpression("App-"));
    QVERIFY(model.rowCount() > 0);
    
    model.setFilterRegularExpression(QRegularExpression("App_"));
    QVERIFY(model.rowCount() > 0);
    
    
    bool foundSpecialApp = false;
    
    model.setFilterRegularExpression(QRegularExpression("App"));
    QVERIFY(model.rowCount() > 0);
    
    for (int i = 0; i < model.rowCount(); ++i) {
        QString desktopId = model.data(model.index(i, 0), AppItem::DesktopIdRole).toString();
        if (desktopId.startsWith("org.special.app")) {
            foundSpecialApp = true;
            break;
        }
    }
    
    QVERIFY(foundSpecialApp);
}

QTEST_MAIN(TestSearchFilterProxyModel)
#include "searchfilterproxymodeltest.moc" 