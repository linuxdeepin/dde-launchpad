// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QLoggingCategory>

#include "itemspage.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")
}

class TestItemsPage: public QObject
{
    Q_OBJECT
private slots:
    void insertAndRemove();
    void autoRemoveEmptyPage();
    void dragItemToFolder();
};

void TestItemsPage::insertAndRemove()
{
    qCInfo(logTest) << "Testing insertAndRemove functionality";
    ItemsPage ip(3);
    qCDebug(logTest) << "Created ItemsPage with max 3 items per page";
    
    ip.appendPage({"a", "b", "c"});
    qCDebug(logTest) << "Appended first page with items a,b,c";
    QVERIFY(ip.pageCount() == 1);
    
    ip.appendPage({"e"});
    qCDebug(logTest) << "Appended second page with item e";
    QVERIFY(ip.pageCount() == 2);
    
    ip.insertItem("d", 1);
    qCDebug(logTest) << "Inserted item d to page 1";
    QCOMPARE(ip.items(0), QStringList({"a", "b", "c"}));
    QCOMPARE(ip.items(1), QStringList({"d", "e"}));
    
    ip.insertItem("^", 0, 1);
    qCDebug(logTest) << "Inserted item ^ at page 0, position 1";
    QCOMPARE(ip.items(0), QStringList({"a", "^", "b"}));
    QCOMPARE(ip.items(1), QStringList({"c", "d", "e"}));
    
    ip.removeItem("d");
    qCDebug(logTest) << "Removed item d";
    QCOMPARE(ip.items(1), QStringList({"c", "e"}));
    qCInfo(logTest) << "insertAndRemove test completed successfully";
}

void TestItemsPage::autoRemoveEmptyPage()
{
    qCInfo(logTest) << "Testing autoRemoveEmptyPage functionality";
    ItemsPage ip(4);
    qCDebug(logTest) << "Created ItemsPage with max 4 items per page";
    
    ip.appendPage({"1", "2", "3"});
    ip.appendPage({"4"});
    ip.appendPage({"5", "6", "7"});
    qCDebug(logTest) << "Created 3 pages with items: [1,2,3], [4], [5,6,7]";

    ip.moveItemPosition(1, 0, 2, 1, true);
    qCDebug(logTest) << "Moved item from page 1, pos 0 to page 2, pos 1";
    QVERIFY(ip.pageCount() == 2);
    qCDebug(logTest) << "Verified page count is 2 after move";

    ip.appendPage({"8"});
    qCDebug(logTest) << "Appended page with item 8";
    ip.moveItemPosition(2, 0, 0, 1, true);
    qCDebug(logTest) << "Moved item from page 2, pos 0 to page 0, pos 1";
    QVERIFY(ip.pageCount() == 2);
    qCInfo(logTest) << "autoRemoveEmptyPage test completed successfully";
}

void TestItemsPage::dragItemToFolder()
{
    qCInfo(logTest) << "Testing dragItemToFolder functionality";
    ItemsPage dstFolder(3);
    qCDebug(logTest) << "Created destination folder with max 3 items per page";
    
    dstFolder.appendPage({"1", "2"});
    qCDebug(logTest) << "Added initial page with items 1,2";

    dstFolder.insertItemToPage("3", -1);
    qCDebug(logTest) << "Inserted item 3 to auto page (-1)";
    
    QVERIFY(dstFolder.pageCount() == 1);
    qCDebug(logTest) << "Verified page count is still 1";
    
    QVERIFY(dstFolder.items(0) == QStringList({"1", "2", "3"}));
    qCInfo(logTest) << "Verified page 0 contains items [1,2,3], dragItemToFolder test completed";
}

QTEST_MAIN(TestItemsPage)
#include "itemspagetest.moc"
