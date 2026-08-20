// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
    void findItemReturnsValidPosition();
    void findItemReturnsInvalidForNonexistent();
    void containsReturnsCorrect();
    void nameSetterAndGetter();
    void itemCountTotalAndPerPage();
    void firstNItemsReturnsUpToN();
    void allArrangedItemsReturnsAll();
    void removeItemsNotInRemovesAbsent();
    void removeEmptyPagesRemovesAllEmpty();
    void appendItemCreatesNewPageWhenFull();
    void moveItemPositionSamePageAdjacentAppendIsNoop();
    void moveItemPositionCrossPage();
    void removeItemNonExistentIsNoop();
    void insertItemToPageAutoSelectsPage();
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

void TestItemsPage::findItemReturnsValidPosition()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b", "c"});
    ip.appendPage({"d"});

    auto [page, index] = ip.findItem("b");
    QCOMPARE(page, 0);
    QCOMPARE(index, 1);

    auto [page2, index2] = ip.findItem("d");
    QCOMPARE(page2, 1);
    QCOMPARE(index2, 0);
}

void TestItemsPage::findItemReturnsInvalidForNonexistent()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b"});

    auto [page, index] = ip.findItem("nonexistent");
    QCOMPARE(page, -1);
    QCOMPARE(index, -1);
}

void TestItemsPage::containsReturnsCorrect()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b"});

    QVERIFY(ip.contains("a"));
    QVERIFY(ip.contains("b"));
    QVERIFY(!ip.contains("c"));
}

void TestItemsPage::nameSetterAndGetter()
{
    ItemsPage ip(3);
    QVERIFY(ip.name().isEmpty());
    ip.setName(QStringLiteral("MyFolder"));
    QCOMPARE(ip.name(), QStringLiteral("MyFolder"));
    ip.setName(QStringLiteral("Renamed"));
    QCOMPARE(ip.name(), QStringLiteral("Renamed"));
}

void TestItemsPage::itemCountTotalAndPerPage()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b", "c"});
    ip.appendPage({"d", "e"});

    QCOMPARE(ip.itemCount(), 5);
    QCOMPARE(ip.itemCount(0), 3);
    QCOMPARE(ip.itemCount(1), 2);
}

void TestItemsPage::firstNItemsReturnsUpToN()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b"});
    ip.appendPage({"c", "d", "e"});

    QCOMPARE(ip.firstNItems(3), QStringList({"a", "b", "c"}));
    QCOMPARE(ip.firstNItems(1), QStringList({"a"}));
    QCOMPARE(ip.firstNItems(5), QStringList({"a", "b", "c", "d", "e"}));
}

void TestItemsPage::allArrangedItemsReturnsAll()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b"});
    ip.appendPage({"c"});

    QCOMPARE(ip.allArrangedItems(), QStringList({"a", "b", "c"}));
}

void TestItemsPage::removeItemsNotInRemovesAbsent()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b", "c"});
    ip.appendPage({"d"});

    QSet<QString> keep = {"a", "c", "d"};
    ip.removeItemsNotIn(keep);

    QCOMPARE(ip.allArrangedItems(), QStringList({"a", "c", "d"}));
}

void TestItemsPage::removeEmptyPagesRemovesAllEmpty()
{
    ItemsPage ip(3);
    ip.appendPage({"a"});
    ip.appendEmptyPage();
    ip.appendEmptyPage();
    QCOMPARE(ip.pageCount(), 3);

    ip.removeEmptyPages();
    QCOMPARE(ip.pageCount(), 1);
    QCOMPARE(ip.items(0), QStringList({"a"}));
}

void TestItemsPage::appendItemCreatesNewPageWhenFull()
{
    ItemsPage ip(2);
    ip.appendPage({"a", "b"}); // page 0 full
    QCOMPARE(ip.pageCount(), 1);

    ip.appendItem("c"); // should create page 1
    QCOMPARE(ip.pageCount(), 2);
    QCOMPARE(ip.items(1), QStringList({"c"}));
}

void TestItemsPage::moveItemPositionSamePageAdjacentAppendIsNoop()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b", "c"});

    // move from page 0, index 1 to page 0, index 0 with append=true
    // since fromIndex(1) > toIndex(0) and append=true, and they're adjacent (1 == 0+1),
    // the source does nothing.
    ip.moveItemPosition(0, 1, 0, 0, true);
    QCOMPARE(ip.items(0), QStringList({"a", "b", "c"}));
}

void TestItemsPage::moveItemPositionCrossPage()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b", "c"});
    ip.appendPage({"d", "e"});

    // move "b" from page 0, index 1 to page 1, index 0
    ip.moveItemPosition(0, 1, 1, 0, false);
    QCOMPARE(ip.items(0), QStringList({"a", "c"}));
    QCOMPARE(ip.items(1), QStringList({"b", "d", "e"}));
}

void TestItemsPage::removeItemNonExistentIsNoop()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b"});
    QCOMPARE(ip.pageCount(), 1);

    ip.removeItem("nonexistent");
    QCOMPARE(ip.pageCount(), 1);
    QCOMPARE(ip.items(0), QStringList({"a", "b"}));
}

void TestItemsPage::insertItemToPageAutoSelectsPage()
{
    ItemsPage ip(3);
    ip.appendPage({"a", "b", "c"}); // page 0 full
    ip.appendPage({"d"});

    // insertItemToPage with page=-1 should auto-select the last page with space
    ip.insertItemToPage("e", -1);
    QVERIFY(ip.contains("e"));
}

QTEST_MAIN(TestItemsPage)
#include "itemspagetest.moc"
