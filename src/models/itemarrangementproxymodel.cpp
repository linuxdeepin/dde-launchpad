// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemarrangementproxymodel.h"

// Presentation adapter only. The dde-apps model owns the folders, pagination,
// persistence and all mutations; the dock reads the very same QObject.
ItemArrangementProxyModel::ItemArrangementProxyModel(QObject *parent)
    : QAbstractListModel(parent)
{
    auto &apps = AppsModel::instance();
    connect(&apps, &QAbstractItemModel::rowsAboutToBeInserted, this, [this](const QModelIndex &, int first, int last) {
        beginInsertRows({}, first, last);
    });
    connect(&apps, &QAbstractItemModel::rowsInserted, this, [this] { endInsertRows(); });
    connect(&apps, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex &, int first, int last) {
        beginRemoveRows({}, first, last);
    });
    connect(&apps, &QAbstractItemModel::rowsRemoved, this, [this] { endRemoveRows(); });
    connect(&apps, &QAbstractItemModel::modelAboutToBeReset, this, [this] { beginResetModel(); });
    connect(&apps, &QAbstractItemModel::modelReset, this, [this] { endResetModel(); });
    connect(&apps, &QAbstractItemModel::dataChanged, this, [this] { refreshArrangement(); });
}

ItemArrangementProxyModel::~ItemArrangementProxyModel() = default;

void ItemArrangementProxyModel::setFolderModel(QAbstractItemModel *model)
{
    if (m_folderModel == model) return;
    if (m_folderModel) disconnect(m_folderModel, nullptr, this, nullptr);
    m_folderModel = model;
    if (model) {
        connect(model, SIGNAL(arrangementChanged()), this, SLOT(refreshArrangement()));
        connect(model, &QAbstractItemModel::rowsInserted, this, &ItemArrangementProxyModel::rebuildFolders);
        connect(model, &QAbstractItemModel::rowsRemoved, this, &ItemArrangementProxyModel::rebuildFolders);
        connect(model, &QAbstractItemModel::modelReset, this, &ItemArrangementProxyModel::rebuildFolders);
        connect(model, &QObject::destroyed, this, [this] { m_folderModel = nullptr; rebuildFolders(); refreshArrangement(); });
    }
    rebuildFolders();
    refreshArrangement();
}

void ItemArrangementProxyModel::rebuildFolders()
{
    QStringList ids;
    if (m_folderModel) {
        const int role = m_folderModel->roleNames().key("desktopId", -1);
        for (int row = 0; row < m_folderModel->rowCount(); ++row) {
            const QString id = m_folderModel->index(row, 0).data(role).toString();
            if (id != "internal/folders/0" && !id.isEmpty()) ids.append(id);
        }
    }
    if (ids == m_folderIds) return;
    // Preserve application indexes and delegates during folder creation/removal.
    const int offset = AppsModel::instance().rowCount();
    for (int i = m_folderIds.size() - 1; i >= 0; --i) {
        if (ids.contains(m_folderIds.at(i))) continue;
        beginRemoveRows({}, offset + i, offset + i);
        m_folderIds.removeAt(i);
        endRemoveRows();
    }
    for (const auto &id : ids) {
        if (m_folderIds.contains(id)) continue;
        beginInsertRows({}, offset + m_folderIds.size(), offset + m_folderIds.size());
        m_folderIds.append(id);
        endInsertRows();
    }
}

int ItemArrangementProxyModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : AppsModel::instance().rowCount() + m_folderIds.size();
}

QVariantMap ItemArrangementProxyModel::folderInfo(const QString &id) const
{
    QVariantMap info;
    if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "folderInfo", Q_RETURN_ARG(QVariantMap, info), Q_ARG(QString, id));
    return info;
}

QVariant ItemArrangementProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) return {};
    const int appCount = AppsModel::instance().rowCount();
    const bool folder = index.row() >= appCount;
    const auto app = folder ? QModelIndex() : AppsModel::instance().index(index.row(), 0);
    const QString id = folder ? m_folderIds.at(index.row() - appCount) : app.data(AppsModel::DesktopIdRole).toString();
    if (role == ItemTypeRole) return folder ? FolderItemType : AppItemType;
    if (role == IconsNameRole) {
        if (!folder) return {};
        QStringList icons;
        const auto contents = folderInfo(id).value("contents").toList();
        for (const auto &entry : contents.mid(0, 4)) icons.append(entry.toMap().value("iconName").toString());
        return icons;
    }
    if (role == PageRole || role == IndexInPageRole || role == FolderIdNumberRole) {
        QVariantMap pos;
        const QString normalized = AppsModel::normalizedDesktopId(id);
        if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "itemPosition", Q_RETURN_ARG(QVariantMap, pos), Q_ARG(QString, normalized));
        return pos.value(role == PageRole ? "page" : role == IndexInPageRole ? "pos" : "group", -1);
    }
    if (!folder) return app.data(role);
    if (role == AppsModel::DesktopIdRole) return id;
    if (role == Qt::DisplayRole || role == AppsModel::NameRole) {
        if (!m_folderModel) return {};
        const auto roles = m_folderModel->roleNames();
        const int idRole = roles.key("desktopId", -1);
        const int nameRole = roles.key("name", -1);
        if (idRole < 0 || nameRole < 0) return {};
        const auto folderIndex = m_folderModel->match(m_folderModel->index(0, 0), idRole, id, 1, Qt::MatchExactly).value(0);
        return folderIndex.data(nameRole);
    }
    if (role == AppsModel::IsAutoStartRole) return false;
    return {};
}

QHash<int, QByteArray> ItemArrangementProxyModel::roleNames() const
{
    auto roles = AppsModel::instance().roleNames();
    roles.insert(IconsNameRole, "folderIcons");
    roles.insert(ItemTypeRole, "itemType");
    return roles;
}

int ItemArrangementProxyModel::pageCount(int folderId) const
{
    int count = 0;
    if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "pageCount", Q_RETURN_ARG(int, count), Q_ARG(int, folderId));
    return count;
}

void ItemArrangementProxyModel::updateFolderName(int folderId, const QString &name)
{
    if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "updateFolderName", Q_ARG(int, folderId), Q_ARG(QString, name));
}

void ItemArrangementProxyModel::bringToFront(const QString &id)
{
    const QString normalized = AppsModel::normalizedDesktopId(id);
    if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "bringToFront", Q_ARG(QString, normalized));
    emit itemBroughtToFront();
}

void ItemArrangementProxyModel::commitDndOperation(const QString &dragId, const QString &dropId, DndOperation op, int pageHint)
{
    if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "rearrange", Q_ARG(QString, dragId), Q_ARG(QString, dropId), Q_ARG(int, int(op)), Q_ARG(int, pageHint));
}

int ItemArrangementProxyModel::creatEmptyPage(int folderId) const
{
    int page = -1;
    if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "createEmptyPage", Q_RETURN_ARG(int, page), Q_ARG(int, folderId));
    return page;
}

void ItemArrangementProxyModel::removeEmptyPage() const
{
    if (m_folderModel) QMetaObject::invokeMethod(m_folderModel, "removeEmptyPages");
}

void ItemArrangementProxyModel::refreshArrangement()
{
    if (rowCount()) emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    emit topLevelPageCountChanged();
    for (const auto &id : m_folderIds) emit folderPageCountChanged(id.section('/', -1).toInt());
}
