// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLoggingCategory>

#include "../src/models/sortproxymodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

// A minimal list model that allows changing data WITHOUT auto-emitting dataChanged,
// so we can control exactly which roles trigger handleDataChanged. This isolates
// the empty-roles path of SortProxyModel::handleDataChanged from the sort-role path.
class ControllableStringListModel : public QAbstractListModel
{
public:
    explicit ControllableStringListModel(const QStringList &items, QObject *parent = nullptr)
        : QAbstractListModel(parent), m_items(items) {}
    int rowCount(const QModelIndex & = QModelIndex()) const override { return m_items.size(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || role != Qt::DisplayRole)
            return {};
        return m_items.value(index.row());
    }
    void setDataSilent(int row, const QString &value) { m_items[row] = value; }
    void emitDataChanged(int row, const QVector<int> &roles = {})
    {
        const QModelIndex idx = index(row, 0);
        emit dataChanged(idx, idx, roles);
    }
private:
    QStringList m_items;
};
}

class TestSortProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void ascendingAndDescendingOrder();
    void mappingBetweenSourceAndProxy();
    void caseSensitivity();
    void sortByCustomRole();
    void rowsInsertedKeepsOrder();
    void rowsRemovedKeepsOrder();
    void dataChangedReorders();
    void replaceSourceModelResets();
    void sortColumnMinusOneRestoresNaturalOrder();
    void setSortColumnToSameIsNoop();
    void indexWithNoSourceModel();
    void indexWithOutOfRangeColumn();
    void dataWithInvalidProxyIndex();
    void mapToSourceWithInvalidIndex();
    void mapFromSourceWithInvalidIndex();
    void mapFromSourceWithParentReturnsEmpty();
    void handleModelReset();
    void removeMultipleConsecutiveRows();
    void dataChangedWithEmptyRolesReorders();
    void dataChangedWithNonSortRoleDoesNotReorder();
    void columnCountWithAndWithoutSource();
    void sortColumnAndSortOrderGetters();
};

void TestSortProxyModel::ascendingAndDescendingOrder()
{
    qCInfo(logTest) << "SortProxyModel should sort ascending and descending by the display role";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("cherry")));
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    source.appendRow(new QStandardItem(QStringLiteral("banana")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);

    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("banana"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));

    proxy.sort(0, Qt::DescendingOrder);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("banana"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
}

void TestSortProxyModel::mappingBetweenSourceAndProxy()
{
    qCInfo(logTest) << "mapFromSource/mapToSource should be consistent with the sort order";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("cherry"))); // source row 0
    source.appendRow(new QStandardItem(QStringLiteral("apple")));  // source row 1
    source.appendRow(new QStandardItem(QStringLiteral("banana"))); // source row 2

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    // expected proxy order: apple(1), banana(2), cherry(0)

    QCOMPARE(proxy.mapFromSource(source.index(0, 0)).row(), 2); // cherry -> proxy 2
    QCOMPARE(proxy.mapFromSource(source.index(1, 0)).row(), 0); // apple -> proxy 0

    QCOMPARE(proxy.mapToSource(proxy.index(0, 0)).row(), 1); // proxy 0 -> apple
    QCOMPARE(proxy.mapToSource(proxy.index(2, 0)).row(), 0); // proxy 2 -> cherry
}

void TestSortProxyModel::caseSensitivity()
{
    qCInfo(logTest) << "SortProxyModel should honor sortCaseSensitivity";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    source.appendRow(new QStandardItem(QStringLiteral("Banana")));
    source.appendRow(new QStandardItem(QStringLiteral("cherry")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    QCOMPARE(proxy.sortCaseSensitivity(), Qt::CaseSensitive);
    proxy.sort(0, Qt::AscendingOrder);
    // case sensitive: 'B'(66) sorts before 'a'(97)
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Banana"));

    proxy.setSortCaseSensitivity(Qt::CaseInsensitive);
    QCOMPARE(proxy.sortCaseSensitivity(), Qt::CaseInsensitive);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("Banana"));
}

void TestSortProxyModel::sortByCustomRole()
{
    qCInfo(logTest) << "SortProxyModel should sort by a custom sortRole";
    constexpr int WeightRole = Qt::UserRole + 1;
    QStandardItemModel source;
    source.setItemRoleNames({{Qt::DisplayRole, QByteArrayLiteral("display")},
                             {WeightRole, QByteArrayLiteral("weight")}});

    auto makeItem = [](const QString &name, int weight) {
        auto item = new QStandardItem(name);
        item->setData(weight, WeightRole);
        return item;
    };
    source.appendRow(makeItem(QStringLiteral("heavy"), 30));
    source.appendRow(makeItem(QStringLiteral("light"), 5));
    source.appendRow(makeItem(QStringLiteral("medium"), 20));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.setSortRole(WeightRole);
    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.sortRole(), WeightRole);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("light"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("medium"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("heavy"));
}

void TestSortProxyModel::rowsInsertedKeepsOrder()
{
    qCInfo(logTest) << "Inserted source rows should appear at their sorted position";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    source.appendRow(new QStandardItem(QStringLiteral("cherry")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.rowCount(), 2);

    QSignalSpy insertedSpy(&proxy, &QAbstractItemModel::rowsInserted);
    source.appendRow(new QStandardItem(QStringLiteral("banana")));
    QVERIFY(insertedSpy.count() >= 1);
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("banana"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));
}

void TestSortProxyModel::rowsRemovedKeepsOrder()
{
    qCInfo(logTest) << "Removed source rows should disappear while keeping the rest sorted";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    source.appendRow(new QStandardItem(QStringLiteral("banana")));
    source.appendRow(new QStandardItem(QStringLiteral("cherry")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);

    QSignalSpy removedSpy(&proxy, &QAbstractItemModel::rowsRemoved);
    source.removeRow(1); // remove "banana"
    QVERIFY(removedSpy.count() >= 1);
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));
}

void TestSortProxyModel::dataChangedReorders()
{
    qCInfo(logTest) << "Changing source data should reorder the proxy";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    source.appendRow(new QStandardItem(QStringLiteral("banana")));
    source.appendRow(new QStandardItem(QStringLiteral("cherry")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    // order: apple, banana, cherry

    source.item(1)->setText(QStringLiteral("zzz"));
    // expected order after re-sort: apple, cherry, zzz
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("zzz"));
}

void TestSortProxyModel::replaceSourceModelResets()
{
    qCInfo(logTest) << "Replacing the source model should reset to the new rows";
    QStandardItemModel sourceA;
    sourceA.appendRow(new QStandardItem(QStringLiteral("zeta")));
    sourceA.appendRow(new QStandardItem(QStringLiteral("alpha")));

    SortProxyModel proxy;
    proxy.setSourceModel(&sourceA);
    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("alpha"));

    QStandardItemModel sourceB;
    sourceB.appendRow(new QStandardItem(QStringLiteral("delta")));
    sourceB.appendRow(new QStandardItem(QStringLiteral("bravo")));
    sourceB.appendRow(new QStandardItem(QStringLiteral("charlie")));
    proxy.setSourceModel(&sourceB);
    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("bravo"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("charlie"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("delta"));
}

void TestSortProxyModel::sortColumnMinusOneRestoresNaturalOrder()
{
    qCInfo(logTest) << "sort(-1) should disable sorting and restore natural source order";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("cherry"))); // source row 0
    source.appendRow(new QStandardItem(QStringLiteral("apple")));  // source row 1
    source.appendRow(new QStandardItem(QStringLiteral("banana"))); // source row 2

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    // ascending: apple(1), banana(2), cherry(0)
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("banana"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));

    // column -1 disables sorting: reorder() falls back to identity (natural source order)
    proxy.sort(-1);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("banana"));
}

void TestSortProxyModel::setSortColumnToSameIsNoop()
{
    qCInfo(logTest) << "setSortColumn to the same column should be a noop (no signal, no reorder)";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("cherry")));
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    source.appendRow(new QStandardItem(QStringLiteral("banana")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.sortColumn(), 0);
    QCOMPARE(proxy.sortOrder(), Qt::AscendingOrder);

    // setSortColumn to the same column -> no signal emitted
    QSignalSpy spy(&proxy, &SortProxyModel::sortColumnChanged);
    proxy.setSortColumn(0);
    QCOMPARE(spy.count(), 0);

    // order unchanged
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
}

void TestSortProxyModel::indexWithNoSourceModel()
{
    qCInfo(logTest) << "index() with no source model should return invalid";
    SortProxyModel proxy;
    QVERIFY(!proxy.index(0, 0).isValid());
    QCOMPARE(proxy.rowCount(), 0);
}

void TestSortProxyModel::indexWithOutOfRangeColumn()
{
    qCInfo(logTest) << "index() with out-of-range column should return invalid";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    QVERIFY(!proxy.index(0, 5).isValid());  // column 5 out of range
    QVERIFY(!proxy.index(0, -1).isValid()); // negative column
}

void TestSortProxyModel::dataWithInvalidProxyIndex()
{
    qCInfo(logTest) << "data() with invalid proxy index should return empty QVariant";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    QVERIFY(!proxy.data(QModelIndex(), Qt::DisplayRole).isValid());
}

void TestSortProxyModel::mapToSourceWithInvalidIndex()
{
    qCInfo(logTest) << "mapToSource() with invalid index should return invalid";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    QVERIFY(!proxy.mapToSource(QModelIndex()).isValid());
}

void TestSortProxyModel::mapFromSourceWithInvalidIndex()
{
    qCInfo(logTest) << "mapFromSource() with invalid index should return invalid";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    QVERIFY(!proxy.mapFromSource(QModelIndex()).isValid());
}

void TestSortProxyModel::mapFromSourceWithParentReturnsEmpty()
{
    qCInfo(logTest) << "mapFromSource() with a parent index should return empty (flat model)";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    // a child index (has parent) should return empty
    QModelIndex childIdx = source.index(0, 0, source.index(0, 0));
    if (childIdx.isValid()) {
        QVERIFY(!proxy.mapFromSource(childIdx).isValid());
    }
}

void TestSortProxyModel::handleModelReset()
{
    qCInfo(logTest) << "source model reset should reset the proxy";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("cherry")));
    source.appendRow(new QStandardItem(QStringLiteral("apple")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.rowCount(), 2);

    // reset source model
    source.clear();
    QCOMPARE(proxy.rowCount(), 0);

    // add items again
    source.appendRow(new QStandardItem(QStringLiteral("banana")));
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
}

void TestSortProxyModel::removeMultipleConsecutiveRows()
{
    qCInfo(logTest) << "removing multiple consecutive source rows should keep proxy sorted";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));
    source.appendRow(new QStandardItem(QStringLiteral("b")));
    source.appendRow(new QStandardItem(QStringLiteral("c")));
    source.appendRow(new QStandardItem(QStringLiteral("d")));
    source.appendRow(new QStandardItem(QStringLiteral("e")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.rowCount(), 5);

    // remove rows 1 and 2 (b and c) in one operation
    source.removeRows(1, 2);
    QCOMPARE(proxy.rowCount(), 3);
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("a"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("d"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("e"));
}

void TestSortProxyModel::dataChangedWithEmptyRolesReorders()
{
    qCInfo(logTest) << "dataChanged with empty roles should trigger reorder (isolated from sort-role path)";
    // Use ControllableStringListModel to change data WITHOUT auto-emitting dataChanged,
    // then emit dataChanged with empty roles to trigger the empty-roles reorder path.
    ControllableStringListModel source({QStringLiteral("apple"),
                                          QStringLiteral("banana"),
                                          QStringLiteral("cherry")});

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    // order: apple(0), banana(1), cherry(2)
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("banana"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));

    // silently change banana to zzz (no dataChanged emitted -> no auto-reorder)
    source.setDataSilent(1, QStringLiteral("zzz"));

    // now emit dataChanged with empty roles -> handleDataChanged sees roles.isEmpty() -> reorder
    source.emitDataChanged(1, {});

    // expected order after reorder: apple(0), cherry(2), zzz(1)
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("cherry"));
    QCOMPARE(proxy.data(proxy.index(2, 0), Qt::DisplayRole).toString(), QStringLiteral("zzz"));
}

void TestSortProxyModel::dataChangedWithNonSortRoleDoesNotReorder()
{
    qCInfo(logTest) << "dataChanged with non-sort role should not reorder";
    constexpr int UserRole1 = Qt::UserRole + 1;
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("apple")));
    source.appendRow(new QStandardItem(QStringLiteral("banana")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::AscendingOrder);
    // order: apple, banana

    // change a non-sort role on apple -> should NOT reorder
    source.item(0)->setData(42, UserRole1);
    emit source.dataChanged(source.index(0, 0), source.index(0, 0), { UserRole1 });

    // order unchanged
    QCOMPARE(proxy.data(proxy.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("apple"));
    QCOMPARE(proxy.data(proxy.index(1, 0), Qt::DisplayRole).toString(), QStringLiteral("banana"));
}

void TestSortProxyModel::columnCountWithAndWithoutSource()
{
    qCInfo(logTest) << "columnCount should return source column count or 0";
    SortProxyModel proxy;
    QCOMPARE(proxy.columnCount(), 0);

    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));
    proxy.setSourceModel(&source);
    QCOMPARE(proxy.columnCount(), 1);
}

void TestSortProxyModel::sortColumnAndSortOrderGetters()
{
    qCInfo(logTest) << "sortColumn and sortOrder getters should return current values";
    QStandardItemModel source;
    source.appendRow(new QStandardItem(QStringLiteral("a")));

    SortProxyModel proxy;
    proxy.setSourceModel(&source);
    QCOMPARE(proxy.sortColumn(), -1); // not yet sorted
    QCOMPARE(proxy.sortOrder(), Qt::AscendingOrder); // default

    proxy.sort(0, Qt::DescendingOrder);
    QCOMPARE(proxy.sortColumn(), 0);
    QCOMPARE(proxy.sortOrder(), Qt::DescendingOrder);
}

QTEST_MAIN(TestSortProxyModel)
#include "sortproxymodeltest.moc"
