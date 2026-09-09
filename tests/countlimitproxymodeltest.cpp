// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>

#include "countlimitproxymodel.h"

namespace {
}

class TestCountLimitProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void noLimitByDefault();
    void maxRowCountLimitsVisibleRows();
    void maxRowCountChangeEmitsSignal();
    void unchangedMaxRowCountDoesNotEmit();
    void sourceRowInsertionReevaluates();
    void sourceRowRemovalReevaluates();
    void maxRowCountEqualsSourceCount();
    void maxRowCountExceedsSourceCount();
    void setSourceModelWithNullDisconnects();
    void setSameSourceModelIsNoop();
};

void TestCountLimitProxyModel::noLimitByDefault()
{
    QStandardItemModel source;
    CountLimitProxyModel proxy;
    QCOMPARE(proxy.maxRowCount(), -1);
    proxy.setSourceModel(&source);

    source.appendRow(new QStandardItem(QStringLiteral("a")));
    source.appendRow(new QStandardItem(QStringLiteral("b")));
    source.appendRow(new QStandardItem(QStringLiteral("c")));

    QCOMPARE(proxy.rowCount(), 3);
}

void TestCountLimitProxyModel::maxRowCountLimitsVisibleRows()
{
    QStandardItemModel source;
    for (int i = 0; i < 5; ++i)
        source.appendRow(new QStandardItem(QStringLiteral("item-%1").arg(i)));

    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setMaxRowCount(3);
    QCOMPARE(proxy.maxRowCount(), 3);
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("item-0"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("item-1"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("item-2"));

    // relaxing the limit again shows everything
    proxy.setMaxRowCount(-1);
    QCOMPARE(proxy.rowCount(), 5);
}

void TestCountLimitProxyModel::maxRowCountChangeEmitsSignal()
{
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));
    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);

    QSignalSpy spy(&proxy, &CountLimitProxyModel::maxRowCountChanged);
    proxy.setMaxRowCount(2);
    QCOMPARE(spy.count(), 1);

    // maxRowCount == 0 rejects every row
    proxy.setMaxRowCount(0);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(proxy.rowCount(), 0);
}

void TestCountLimitProxyModel::unchangedMaxRowCountDoesNotEmit()
{
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));
    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setMaxRowCount(2);

    QSignalSpy spy(&proxy, &CountLimitProxyModel::maxRowCountChanged);
    proxy.setMaxRowCount(2);
    QCOMPARE(spy.count(), 0);
}

void TestCountLimitProxyModel::sourceRowInsertionReevaluates()
{
    QStandardItemModel source;
    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setMaxRowCount(2);
    QCOMPARE(proxy.rowCount(), 0);

    for (int i = 0; i < 4; ++i)
        source.appendRow(new QStandardItem(QStringLiteral("item-%1").arg(i)));

    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("item-0"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("item-1"));
}

void TestCountLimitProxyModel::sourceRowRemovalReevaluates()
{
    QStandardItemModel source;
    for (int i = 0; i < 4; ++i)
        source.appendRow(new QStandardItem(QStringLiteral("item-%1").arg(i)));

    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setMaxRowCount(2);
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("item-0"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("item-1"));

    // setSourceModel connects rowsRemoved -> invalidate(), so removing a source row
    // re-runs the filter against the (shifted) remaining rows: item-1, item-2 survive.
    source.removeRow(0);
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("item-1"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("item-2"));
}

void TestCountLimitProxyModel::maxRowCountEqualsSourceCount()
{
    QStandardItemModel source;
    for (int i = 0; i < 3; ++i)
        source.appendRow(new QStandardItem(QStringLiteral("item-%1").arg(i)));

    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setMaxRowCount(3);
    QCOMPARE(proxy.rowCount(), 3); // all visible
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("item-0"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("item-2"));
}

void TestCountLimitProxyModel::maxRowCountExceedsSourceCount()
{
    QStandardItemModel source;
    for (int i = 0; i < 2; ++i)
        source.appendRow(new QStandardItem(QStringLiteral("item-%1").arg(i)));

    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setMaxRowCount(100);
    QCOMPARE(proxy.rowCount(), 2); // all visible, no extra rows
}

void TestCountLimitProxyModel::setSourceModelWithNullDisconnects()
{
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setMaxRowCount(1);
    QCOMPARE(proxy.rowCount(), 1);

    // set null source model
    proxy.setSourceModel(nullptr);
    QCOMPARE(proxy.rowCount(), 0);

    // old model changes should not affect proxy anymore
    source.appendRow(new QStandardItem(QStringLiteral("b")));
    QCOMPARE(proxy.rowCount(), 0);
}

void TestCountLimitProxyModel::setSameSourceModelIsNoop()
{
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    CountLimitProxyModel proxy;
    proxy.setSourceModel(&source);
    QCOMPARE(proxy.rowCount(), 1);

    // set same model again
    proxy.setSourceModel(&source);
    QCOMPARE(proxy.rowCount(), 1);
}

QTEST_MAIN(TestCountLimitProxyModel)
#include "countlimitproxymodeltest.moc"
