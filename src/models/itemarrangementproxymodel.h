// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>

#include <QPointer>
#include "appsmodel.h"

#include <QAbstractListModel>
#include <QStandardItemModel>

class ItemArrangementProxyModel : public QAbstractListModel
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

    void setFolderModel(QAbstractItemModel *model);
    int rowCount(const QModelIndex &parent = {}) const override;

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void topLevelPageCountChanged();
    void folderPageCountChanged(int folderId);
    void itemBroughtToFront();

private:
    explicit ItemArrangementProxyModel(QObject *parent = nullptr);

private slots:
    void refreshArrangement();

private:
    QVariantMap folderInfo(const QString &id) const;
    void rebuildFolders();
    QPointer<QAbstractItemModel> m_folderModel;
    QStringList m_folderIds;
};
