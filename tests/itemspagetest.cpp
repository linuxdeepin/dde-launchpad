// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>

#include "itemspage.h"

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
    ItemsPage ip(3);
    ip.appendPage({"a", "b", "c"});
    QVERIFY(ip.pageCount() == 1);
    ip.appendPage({"e"});
    QVERIFY(ip.pageCount() == 2);
    ip.insertItem("d", 1);
    QCOMPARE(ip.items(0), QStringList({"a", "b", "c"}));
    QCOMPARE(ip.items(1), QStringList({"d", "e"}));
    ip.insertItem("^", 0, 1);
    QCOMPARE(ip.items(0), QStringList({"a", "^", "b"}));
    QCOMPARE(ip.items(1), QStringList({"c", "d", "e"}));
    ip.removeItem("d");
    QCOMPARE(ip.items(1), QStringList({"c", "e"}));
}

void TestItemsPage::autoRemoveEmptyPage()
{
    ItemsPage ip(4);
    ip.appendPage({"1", "2", "3"});
    ip.appendPage({"4"});
    ip.appendPage({"5", "6", "7"});

    ip.moveItemPosition(1, 0, 2, 1, true);
    QVERIFY(ip.pageCount() == 2);

    ip.appendPage({"8"});
    ip.moveItemPosition(2, 0, 0, 1, true);
    QVERIFY(ip.pageCount() == 2);
}

void TestItemsPage::dragItemToFolder()
{
    ItemsPage dstFolder(3);
    dstFolder.appendPage({"1", "2"});

    dstFolder.insertItemToPage("3", -1);
    QVERIFY(dstFolder.pageCount() == 1);
    QVERIFY(dstFolder.items(0) == QStringList({"1", "2", "3"}));
}

QTEST_MAIN(TestItemsPage)
#include "itemspagetest.moc"
