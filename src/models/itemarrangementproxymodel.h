// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>

#include "itemspage.h"
#include "appsmodel.h"

#include <QConcatenateTablesProxyModel>

class ItemArrangementProxyModel : public QConcatenateTablesProxyModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ItemArrangementProxyModel)
    QML_SINGLETON
public:
    enum ItemType{
        AppItemType = 0,
        FolderItemType = 1
    };
    Q_ENUM(ItemType)

    enum Roles {
        PageRole = AppsModel::ProxyModelExtendedRole,
        IndexInPageRole,
        FolderIdNumberRole,
        IconsNameRole,
        ItemTypeRole
    };
    Q_ENUM(Roles)

    enum DndOperation {
        DndPrepend = -1,
        DndJoin = 0,
        DndAppend = 1
    };
    Q_ENUM(DndOperation)

    static ItemArrangementProxyModel &instance()
    {
        static ItemArrangementProxyModel _instance;
        return _instance;
    }

    static ItemArrangementProxyModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &instance();
    }

    ~ItemArrangementProxyModel();

    Q_INVOKABLE int pageCount(int folderId = 0) const;
    Q_INVOKABLE void updateFolderName(int folderId, const QString & name);
    Q_INVOKABLE void bringToFront(const QString & id);
    Q_INVOKABLE void commitDndOperation(const QString & dragId, const QString & dropId, const DndOperation op, int pageHint = -1);
    Q_INVOKABLE int creatEmptyPage(int folderId = 0) const;
    Q_INVOKABLE void removeEmptyPage() const;

    ItemsPage *itemsPage() { return m_topLevel; }

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void topLevelPageCountChanged();
    void folderPageCountChanged(int folderId);

private:
    explicit ItemArrangementProxyModel(QObject *parent = nullptr);

    void loadItemArrangementFromUserData();
    void saveItemArrangementToUserData();
    std::tuple<int, int, int> findItem(const QString & id, bool searchTopLevelOnly = false) const;
    void onSourceModelChanged();
    void onFolderModelChanged();

    QString findAvailableFolderId();
    ItemsPage * createFolder(const QString & id);
    void removeFolder(const QString & idNumber);
    ItemsPage * folderById(int id);
    QStringList allArrangedItems() const;

    // <folder-id, items-arrangement-data> folder-id: internal/folder/<id number>
    ItemsPage * m_topLevel;
    QHash<QString, ItemsPage *> m_folders;
    QStandardItemModel m_folderModel;
};
