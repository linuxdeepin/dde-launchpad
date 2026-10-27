// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/models/itemarrangementproxymodel.h"
#include "../src/models/itemspagemodel.h"
#include <QAbstractItemModelTester>
#include <QPersistentModelIndex>
#include <QTest>

// The adapter must only present and forward to the shared provider. It must not
// read/write arrangement settings or depend on the provider's concrete C++ type.
class FolderProvider : public QStandardItemModel
{
    Q_OBJECT
public:
    int pages = 1;
    QString renamed;
    QStringList lastMove;
    int contentsQueries = 0;
    FolderProvider()
    {
        setItemRoleNames({{Qt::UserRole, "desktopId"}, {Qt::UserRole + 1, "name"}});
        for (const auto &id : {"internal/folders/0", "internal/folders/12", "internal/folders/20"}) {
            auto item = new QStandardItem;
            item->setData(QString(id), Qt::UserRole);
            item->setData(QString(id), Qt::UserRole + 1);
            appendRow(item);
        }
    }
    Q_INVOKABLE QVariantMap folderInfo(const QString &id)
    {
        ++contentsQueries;
        return {{"id", id}, {"name", renamed.isEmpty() ? id : renamed},
                {"contents", QVariantList{QVariantMap{{"id", "a"}, {"name", "A"}, {"iconName", "folder"}}}}};
    }
    Q_INVOKABLE QVariantMap itemPosition(const QString &id)
    {
        return {{"group", id.startsWith("a") ? 12 : 0}, {"page", 0}, {"pos", 0}};
    }
    Q_INVOKABLE int pageCount(int) { return pages; }
    Q_INVOKABLE void updateFolderName(int folderId, const QString &name) {
        renamed = name;
        const auto folder = match(index(0, 0), Qt::UserRole, QStringLiteral("internal/folders/%1").arg(folderId), 1, Qt::MatchExactly).value(0);
        setData(folder, name, Qt::UserRole + 1);
        emit arrangementChanged();
    }
    Q_INVOKABLE void rearrange(const QString &drag, const QString &drop, int, int) { lastMove = {drag, drop}; }
signals:
    void arrangementChanged();
};

class ItemArrangementProxyTest : public QObject
{
    Q_OBJECT
    QStandardItemModel applications;
    FolderProvider *provider = nullptr;
private slots:
    void init()
    {
        applications.setItemRoleNames({{Qt::UserRole, "desktopId"}, {Qt::UserRole + 1, "name"}, {Qt::UserRole + 2, "iconName"}});
        for (const auto &id : {"a", "b"}) {
            auto item = new QStandardItem;
            item->setData(QString(id), Qt::UserRole);
            item->setData(QString(id).toUpper(), Qt::UserRole + 1);
            item->setData("folder", Qt::UserRole + 2);
            applications.appendRow(item);
        }
        AppsModel::instance().setSourceModel(&applications);
        provider = new FolderProvider;
        ItemArrangementProxyModel::instance().setFolderModel(provider);
    }
    void cleanup()
    {
        ItemArrangementProxyModel::instance().setFolderModel(nullptr);
        delete provider;
        provider = nullptr;
        AppsModel::instance().setSourceModel(nullptr);
        applications.clear();
    }
    void sharedIdentityAndForwarding()
    {
        auto &adapter = ItemArrangementProxyModel::instance();
        QAbstractItemModelTester tester(&adapter, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QCOMPARE(adapter.rowCount(), 4);
        QCOMPARE(adapter.index(0, 0).data(ItemArrangementProxyModel::FolderIdNumberRole).toInt(), 12);
        QCOMPARE(adapter.index(2, 0).data(AppsModel::DesktopIdRole).toString(), "internal/folders/12");
        QCOMPARE(adapter.index(2, 0).data(ItemArrangementProxyModel::IconsNameRole).toStringList(), QStringList{"folder"});
        adapter.updateFolderName(12, "Renamed");
        QCOMPARE(provider->renamed, "Renamed");
        QCOMPARE(adapter.index(2, 0).data(AppsModel::NameRole).toString(), "Renamed");
        adapter.commitDndOperation("a", "internal/folders/20", ItemArrangementProxyModel::DndJoin);
        QCOMPARE(provider->lastMove, QStringList({"a", "internal/folders/20"}));
        QPersistentModelIndex remaining(adapter.index(3, 0));
        provider->removeRow(1);
        QCOMPARE(adapter.rowCount(), 3);
        QCOMPARE(remaining.row(), 2);
        QCOMPARE(remaining.data(AppsModel::DesktopIdRole).toString(), "internal/folders/20");
    }
    void appRemovalAndProviderLifetime()
    {
        auto &adapter = ItemArrangementProxyModel::instance();
        QAbstractItemModelTester tester(&adapter, QAbstractItemModelTester::FailureReportingMode::QtTest);
        QPersistentModelIndex folder(adapter.index(2, 0));
        applications.removeRow(0);
        QCOMPARE(adapter.rowCount(), 3);
        QCOMPARE(folder.row(), 1);
        QCOMPARE(folder.data(AppsModel::DesktopIdRole).toString(), "internal/folders/12");
        ItemsPageModel pages;
        QAbstractItemModelTester pageTester(&pages, QAbstractItemModelTester::FailureReportingMode::QtTest);
        pages.setSourceModel(&adapter);
        QCOMPARE(pages.rowCount(), 1);
        provider->pages = 3;
        emit provider->arrangementChanged();
        QCOMPARE(pages.rowCount(), 3);
        delete provider;
        provider = nullptr;
        QCOMPARE(adapter.rowCount(), 1);
        QCOMPARE(pages.rowCount(), 0);
    }
    void folderNameDoesNotReadContents()
    {
        auto &adapter = ItemArrangementProxyModel::instance();
        const auto folder = adapter.index(2, 0);
        provider->contentsQueries = 0;
        QCOMPARE(folder.data(AppsModel::NameRole).toString(), "internal/folders/12");
        QCOMPARE(folder.data(Qt::DisplayRole).toString(), "internal/folders/12");
        QCOMPARE(provider->contentsQueries, 0);
        QCOMPARE(folder.data(ItemArrangementProxyModel::IconsNameRole).toStringList(), QStringList{"folder"});
        QCOMPARE(provider->contentsQueries, 1);
    }
};
QTEST_MAIN(ItemArrangementProxyTest)
#include "itemarrangementproxytest.moc"
