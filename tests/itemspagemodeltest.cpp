// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLoggingCategory>

#include "../src/models/itemspagemodel.h"
#include "../src/models/itemarrangementproxymodel.h"
#include "../src/models/appsmodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

enum SourceRoles {
    SrcDesktopIdRole = Qt::UserRole + 1,
    SrcNameRole,
    SrcIconNameRole,
    SrcNoDisplayRole,
    SrcDDECategoryRole,
    SrcInstalledTimeRole,
    SrcLastLaunchedTimeRole,
    SrcLaunchedTimesRole,
    SrcAutoStartRole,
    SrcCategoriesRole,
    SrcVendorRole,
    SrcGenericNameRole,
};

QStandardItem *makeApp(const QString &desktopId, const QString &name)
{
    auto item = new QStandardItem;
    item->setData(desktopId, SrcDesktopIdRole);
    item->setData(name, SrcNameRole);
    item->setData(QStringLiteral("application-default-icon"), SrcIconNameRole);
    item->setData(false, SrcNoDisplayRole);
    item->setData(0, SrcDDECategoryRole);
    return item;
}
}

class TestItemsPageModel : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void rowCountIsZeroWithoutSourceModel();
    void dataReturnsEmptyVariant();
    void setSourceModelWithNullIsNoop();
    void setSameSourceModelIsNoop();
    void setSourceModelConnectsToItemsPage();
    void rowCountReflectsItemsPageCount();
    void sigPageAddedInsertsRows();
    void sigPageRemovedRemovesRows();
    void sourceModelGetterReturnsSetModel();

private:
    QStandardItemModel m_sourceModel;
};

void TestItemsPageModel::initTestCase()
{
    m_sourceModel.setItemRoleNames({
        { SrcDesktopIdRole, QByteArrayLiteral("desktopId") },
        { SrcNameRole, QByteArrayLiteral("name") },
        { SrcIconNameRole, QByteArrayLiteral("iconName") },
        { SrcNoDisplayRole, QByteArrayLiteral("noDisplay") },
        { SrcDDECategoryRole, QByteArrayLiteral("ddeCategory") },
        { SrcInstalledTimeRole, QByteArrayLiteral("installedTime") },
        { SrcLastLaunchedTimeRole, QByteArrayLiteral("lastLaunchedTime") },
        { SrcLaunchedTimesRole, QByteArrayLiteral("launchedTimes") },
        { SrcAutoStartRole, QByteArrayLiteral("autoStart") },
        { SrcCategoriesRole, QByteArrayLiteral("categories") },
        { SrcVendorRole, QByteArrayLiteral("vendor") },
        { SrcGenericNameRole, QByteArrayLiteral("genericName") },
    });
    m_sourceModel.appendRow(makeApp("page-a.desktop", "Page A"));
    m_sourceModel.appendRow(makeApp("page-b.desktop", "Page B"));
    m_sourceModel.appendRow(makeApp("page-c.desktop", "Page C"));

    AppsModel::instance().setSourceModel(&m_sourceModel);
    AppsModel::instance().setReady(true);

    // Accessing the singleton triggers construction + onSourceModelChanged,
    // which adds all AppsModel items to topLevel pages.
    ItemArrangementProxyModel::instance();
}

void TestItemsPageModel::rowCountIsZeroWithoutSourceModel()
{
    ItemsPageModel model;
    QCOMPARE(model.rowCount(), 0);
}

void TestItemsPageModel::dataReturnsEmptyVariant()
{
    ItemsPageModel model;
    // data() always returns empty QVariant (per source)
    QVERIFY(!model.data(model.index(0, 0)).isValid());
}

void TestItemsPageModel::setSourceModelWithNullIsNoop()
{
    ItemsPageModel model;
    QSignalSpy spy(&model, &ItemsPageModel::sourceModelChanged);
    model.setSourceModel(nullptr);
    QCOMPARE(spy.count(), 0); // null model -> skip, no signal
    QCOMPARE(model.rowCount(), 0);
}

void TestItemsPageModel::setSameSourceModelIsNoop()
{
    ItemsPageModel model;
    auto &iapm = ItemArrangementProxyModel::instance();

    model.setSourceModel(&iapm);
    QSignalSpy spy(&model, &ItemsPageModel::sourceModelChanged);
    model.setSourceModel(&iapm); // same model -> skip
    QCOMPARE(spy.count(), 0);
}

void TestItemsPageModel::setSourceModelConnectsToItemsPage()
{
    ItemsPageModel model;
    auto &iapm = ItemArrangementProxyModel::instance();

    QSignalSpy spy(&model, &ItemsPageModel::sourceModelChanged);
    model.setSourceModel(&iapm);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.sourceModel(), &iapm);
}

void TestItemsPageModel::rowCountReflectsItemsPageCount()
{
    ItemsPageModel model;
    auto &iapm = ItemArrangementProxyModel::instance();
    model.setSourceModel(&iapm);

    // rowCount should match the topLevel pageCount (apps added in initTestCase)
    QCOMPARE(model.rowCount(), iapm.itemsPage()->pageCount());
    QVERIFY(model.rowCount() > 0);
}

void TestItemsPageModel::sigPageAddedInsertsRows()
{
    ItemsPageModel model;
    auto &iapm = ItemArrangementProxyModel::instance();
    model.setSourceModel(&iapm);

    int before = model.rowCount();
    // create an empty page -> sigPageAdded -> beginInsertRows/endInsertRows
    QSignalSpy insertSpy(&model, &QAbstractItemModel::rowsInserted);
    iapm.creatEmptyPage(0);
    QVERIFY(insertSpy.count() >= 1);
    QCOMPARE(model.rowCount(), before + 1);

    // cleanup
    iapm.removeEmptyPage();
    QCOMPARE(model.rowCount(), before);
}

void TestItemsPageModel::sigPageRemovedRemovesRows()
{
    ItemsPageModel model;
    auto &iapm = ItemArrangementProxyModel::instance();
    model.setSourceModel(&iapm);

    int before = model.rowCount();
    // create an empty page first
    iapm.creatEmptyPage(0);
    QCOMPARE(model.rowCount(), before + 1);

    // remove it -> sigPageRemoved -> beginRemoveRows/endRemoveRows
    QSignalSpy removeSpy(&model, &QAbstractItemModel::rowsRemoved);
    iapm.removeEmptyPage();
    QVERIFY(removeSpy.count() >= 1);
    QCOMPARE(model.rowCount(), before);
}

void TestItemsPageModel::sourceModelGetterReturnsSetModel()
{
    ItemsPageModel model;
    auto &iapm = ItemArrangementProxyModel::instance();
    model.setSourceModel(&iapm);
    QCOMPARE(model.sourceModel(), &iapm);
}

QTEST_MAIN(TestItemsPageModel)
#include "itemspagemodeltest.moc"
