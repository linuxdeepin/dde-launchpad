// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QStandardItem>
#include <QStandardItemModel>

#include "freesortproxymodel.h"
#include "itemarrangementproxymodel.h"

namespace {
}

class TestFreeSortProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void sortByPageThenIndex();
    void descendingOrder();
    void missingRolesDefaultToZero();
};

static QStandardItem *makeArrangedItem(const QString &name, int page, int indexInPage)
{
    auto item = new QStandardItem(name);
    item->setData(page, ItemArrangementProxyModel::PageRole);
    item->setData(indexInPage, ItemArrangementProxyModel::IndexInPageRole);
    return item;
}

void TestFreeSortProxyModel::sortByPageThenIndex()
{
    QStandardItemModel source;
    // intentionally appended out of order so the proxy actually has to sort
    source.appendRow(makeArrangedItem(QStringLiteral("A"), 1, 0));
    source.appendRow(makeArrangedItem(QStringLiteral("B"), 0, 1));
    source.appendRow(makeArrangedItem(QStringLiteral("C"), 0, 0));
    source.appendRow(makeArrangedItem(QStringLiteral("D"), 1, 1));

    FreeSortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);

    QCOMPARE(proxy.rowCount(), 4);
    // ascending: C(0,0), B(0,1), A(1,0), D(1,1)
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("C"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("B"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("A"));
    QCOMPARE(proxy.data(proxy.index(3, 0), Qt::DisplayRole).toString(), QStringLiteral("D"));
}

void TestFreeSortProxyModel::descendingOrder()
{
    QStandardItemModel source;
    source.appendRow(makeArrangedItem(QStringLiteral("A"), 1, 0));
    source.appendRow(makeArrangedItem(QStringLiteral("B"), 0, 1));
    source.appendRow(makeArrangedItem(QStringLiteral("C"), 0, 0));
    source.appendRow(makeArrangedItem(QStringLiteral("D"), 1, 1));

    FreeSortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::DescendingOrder);

    // descending: D(1,1), A(1,0), B(0,1), C(0,0)
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("D"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("A"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("B"));
    QCOMPARE(proxy.data(proxy.index(3, 0), Qt::DisplayRole).toString(), QStringLiteral("C"));
}

void TestFreeSortProxyModel::missingRolesDefaultToZero()
{
    QStandardItemModel source;
    // items without PageRole or IndexInPageRole data -> toInt() returns 0 for both
    source.appendRow(new QStandardItem(QStringLiteral("X")));
    source.appendRow(new QStandardItem(QStringLiteral("Y")));
    source.appendRow(new QStandardItem(QStringLiteral("Z")));

    FreeSortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);

    // all items have page=0, index=0 -> equal keys -> stable source order
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("X"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Y"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("Z"));
}

QTEST_MAIN(TestFreeSortProxyModel)
#include "freesortproxymodeltest.moc"
