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

QTEST_MAIN(TestItemsPage)
#include "itemspagetest.moc"
