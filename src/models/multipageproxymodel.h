// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "itemspage.h"
#include "appsmodel.h"

#include <QIdentityProxyModel>

class MultipageProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    enum Roles {
        PageRole = AppsModel::ProxyModelExtendedRole,
        IndexInPageRole,
        FolderIdNumberRole,
        IconsNameRole
    };
    Q_ENUM(Roles)

    enum DndOperation {
        DndPrepend = -1,
        DndJoin = 0,
        DndAppend = 1
    };
    Q_ENUM(DndOperation)

    static MultipageProxyModel &instance()
    {
        static MultipageProxyModel _instance;
        return _instance;
    }

    ~MultipageProxyModel();

    Q_INVOKABLE int pageCount(int folderId = 0) const;
    Q_INVOKABLE void commitDndOperation(const QString & dragId, const QString & dropId, const DndOperation op);

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

private:
    explicit MultipageProxyModel(QObject *parent = nullptr);

    void loadItemArrangementFromUserData();
    void saveItemArrangementToUserData();
    std::tuple<int, int, int> findItem(const QString & id, bool searchTopLevelOnly = false) const;
    void onSourceModelChanged();

    ItemsPage * createFolder(const QString & idNumber);
    ItemsPage * folderById(int id);

    // <folder-id, items-arrangement-data> folder-id: internal/folder/<id number>
    ItemsPage * m_topLevel;
    QHash<QString, ItemsPage *> m_folders;

};
