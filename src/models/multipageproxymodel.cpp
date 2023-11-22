// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multipageproxymodel.h"

#include "appsmodel.h"
#include "categoryutils.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

MultipageProxyModel::~MultipageProxyModel()
{

}

int MultipageProxyModel::pageCount(int folderId) const
{
    qDebug() << m_topLevel->pageCount() << folderId << "xxxxxxxx";
    if (folderId == 0) return m_topLevel->pageCount();

    QString fullId("internal/folders/" + QString::number(folderId));
    Q_ASSERT(m_folders.contains(fullId));

    return m_folders.value(fullId)->pageCount();
}

void MultipageProxyModel::commitDndOperation(const QString &dragId, const QString &dropId, const DndOperation op)
{
    if (dragId == dropId) return;

    std::tuple<int, int, int> dragOrigPos = findItem(dragId);
    std::tuple<int, int, int> dropOrigPos = findItem(dropId);

    if (op != DndOperation::DndJoin) {
        // move to dropId's front or back
        // DnD can only happen in the same folder
        Q_ASSERT(std::get<0>(dragOrigPos) == std::get<0>(dropOrigPos));
        ItemsPage * folder = folderById(std::get<0>(dropOrigPos));
        const int dragOrigPage = std::get<1>(dragOrigPos);
        const int dropOrigPage = std::get<1>(dropOrigPos);
        // FIXME: drop position not correct
        folder->moveItem(dragOrigPage, std::get<2>(dragOrigPos), dropOrigPage, std::get<2>(dropOrigPos));
    } else {
        if (dragId.startsWith("internal/folders/")) return; // cannot drag folder onto something
        if (std::get<0>(dropOrigPos) != 0) return; // folder inside folder is not allowed
        if (dropId.startsWith("internal/folders/")) {
            // drop into existing folder
            m_topLevel->removeItem(dragId);
            m_folders.value(dropId)->appendItem(dragId);
        } else {
            // make a new folder, move two items into the folder
            int folderCount = m_folders.count();
            QString folderNumStr(QString::number(folderCount + 1));
            ItemsPage * folder = createFolder(folderNumStr);
            folder->appendPage({dragId, dropId});
            AppItem * dropItem = AppsModel::instance().itemFromDesktopId(dropId);
            AppItem::DDECategories dropCategories = AppItem::DDECategories(CategoryUtils::parseBestMatchedCategory(dropItem->categories()));
            folder->setName("internal/category/" + QString::number(dropCategories));
            m_topLevel->removeItem(dragId);
            m_topLevel->removeItem(dropId);
            m_topLevel->insertItem("internal/folders/" + folderNumStr, std::get<1>(dropOrigPos), std::get<2>(dropOrigPos));
        }
    }

    saveItemArrangementToUserData();
    // Lazy solution, just notify the view that all rows and its roles are changed so they need to be updated.
    emit dataChanged(index(0, 0, QModelIndex()), index(rowCount(QModelIndex()), 0, QModelIndex()), {
        PageRole, IndexInPageRole, FolderIdNumberRole, IconsNameRole
    });
}

QModelIndex MultipageProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= sourceModel()->rowCount()) {
        return createIndex(row, column, -1);
    }
    return QIdentityProxyModel::index(row, column, parent);
}

int MultipageProxyModel::rowCount(const QModelIndex &parent) const
{
    return (sourceModel() ? sourceModel()->rowCount(parent) : 0) + m_folders.count();
}

QVariant MultipageProxyModel::data(const QModelIndex &index, int role) const
{
    int idx = index.row() - sourceModel()->rowCount();
    if (idx < 0 && role < AppsModel::ProxyModelExtendedRole) return QIdentityProxyModel::data(index, role);

    if (idx < 0) {
        // regular applications, not a folder
        QString id(data(index, AppItem::DesktopIdRole).toString());
        if (id.isEmpty() || id.contains("internal")) {
            qDebug() << index << index.row() << sourceModel()->rowCount();
        }
        int folder, page, idx;
        std::tie(folder, page, idx) = findItem(id);

        switch (role) {
        case PageRole:
            return page;
        case IndexInPageRole:
            return idx;
        case FolderIdNumberRole:
            return folder;
        case IconsNameRole:
            return QVariant();
        }
    } else {
        // a folder
        QString id("internal/folders/" + QString::number(idx + 1));
        int folder, page, idx;
        if (role >= AppsModel::ProxyModelExtendedRole && role != IconsNameRole) {
            std::tie(folder, page, idx) = findItem(id, true);
        }

        switch (role) {
        case Qt::DisplayRole:
            return m_folders.value(id)->name();
        case AppItem::DesktopIdRole:
            return id;
        case PageRole:
            return page;
        case IndexInPageRole:
            return idx;
        case FolderIdNumberRole:
            return folder;
        case IconsNameRole: {
            const QStringList desktopIds = m_folders.value(id)->firstNItems(4);
            QStringList icons;
            for (const QString & id : desktopIds) {
                AppItem * item = AppsModel::instance().itemFromDesktopId(id);
                if (item) {
                    icons.append(item->iconName());
                }
            }
            return icons;//QStringList({"deepin-music"});
        }
        }
    }

    return QIdentityProxyModel::data(index, role);
}

QHash<int, QByteArray> MultipageProxyModel::roleNames() const
{
    auto existingRoleNames = QIdentityProxyModel::roleNames();
    existingRoleNames.insert(IconsNameRole, QByteArrayLiteral("folderIcons"));
    return existingRoleNames;
}

QModelIndex MultipageProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
//    qDebug() << "toSource" << proxyIndex;

    // any rows beyond the source model is a virtual "folder" item
    if (proxyIndex.row() >= sourceModel()->rowCount()) return QModelIndex();

    return QIdentityProxyModel::mapToSource(proxyIndex);
}

QModelIndex MultipageProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
//    qDebug() << "fromSource";
    return QIdentityProxyModel::mapFromSource(sourceIndex);
}

MultipageProxyModel::MultipageProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , m_topLevel(new ItemsPage(7 * 4, this))
{
//    ItemsPage ip(3);
//    ip.appendPage({"a", "b", "c"});
//    ip.appendPage({"e"});
//    ip.insertItem("d", 1);
//    ip.insertItem("^", 0, 1);
//    ip.removeItem("d");
//    qDebug() << ip.items(0);
//    qDebug() << ip.items(1);

    connect(this, &QAbstractProxyModel::sourceModelChanged, this, &MultipageProxyModel::onSourceModelChanged);

    loadItemArrangementFromUserData();
    setSourceModel(&AppsModel::instance());
}

void MultipageProxyModel::loadItemArrangementFromUserData()
{
    const QString arrangementSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString arrangementSettingPath(QDir(arrangementSettingBasePath).absoluteFilePath("item-arrangement.ini"));
    QSettings itemArrangementSettings(arrangementSettingPath, QSettings::NativeFormat);

    itemArrangementSettings.beginGroup("fullscreen");
    const QStringList folderGroups(itemArrangementSettings.childGroups());

    for (const QString & groupName : folderGroups) {
        itemArrangementSettings.beginGroup(groupName);
        QString folderName = itemArrangementSettings.value("name", QString()).toString();
        int pageCount = itemArrangementSettings.value("pageCount", 0).toInt();
        bool isTopLevel = groupName == "toplevel";

        qDebug() << folderName << pageCount;

        ItemsPage * page = isTopLevel ? m_topLevel : createFolder(groupName);
        page->setName(folderName);

        for (int i = 0; i < pageCount; i++) {
            QStringList items = itemArrangementSettings.value(QString::asprintf("pageItems/%d", i)).toStringList();
            page->appendPage(items);
        }

        itemArrangementSettings.endGroup();
    }
}

void MultipageProxyModel::saveItemArrangementToUserData()
{
    const QString arrangementSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString arrangementSettingPath(QDir(arrangementSettingBasePath).absoluteFilePath("item-arrangement.ini"));
    QSettings itemArrangementSettings(arrangementSettingPath, QSettings::NativeFormat);

    itemArrangementSettings.beginGroup("fullscreen/toplevel");
    int pageCount = m_topLevel->pageCount();
    itemArrangementSettings.setValue("pageCount", pageCount);
    for (int i = 0; i < pageCount; i++) {
        itemArrangementSettings.setValue(QString::asprintf("pageItems/%d", i), m_topLevel->items(i));
    }
    itemArrangementSettings.endGroup();

    for (int i = 1; i <= m_folders.count(); i++) {
        itemArrangementSettings.beginGroup("fullscreen/" + QString::number(i));
        ItemsPage * page = m_folders.value("internal/folders/" + QString::number(i));
        int pageCount = page->pageCount();
        itemArrangementSettings.setValue("name", page->name());
        itemArrangementSettings.setValue("pageCount", pageCount);
        for (int i = 0; i < pageCount; i++) {
            itemArrangementSettings.setValue(QString::asprintf("pageItems/%d", i), page->items(i));
        }
        itemArrangementSettings.endGroup();
    }

    itemArrangementSettings.sync();
}

std::tuple<int, int, int> MultipageProxyModel::findItem(const QString &id, bool searchTopLevelOnly) const
{
    int page, idx;

    std::tie(page, idx) = m_topLevel->findItem(id);
    if (page != -1) return std::make_tuple(0, page, idx);

    if (!searchTopLevelOnly) {
        for (int i = 1; i <= m_folders.count(); i++) {
            std::tie(page, idx) = m_folders["internal/folders/" + QString::number(i)]->findItem(id);
            if (page != -1) return std::make_tuple(i, page, idx);
        }
    }

    return std::make_tuple(-1, -1, -1);
}

void MultipageProxyModel::onSourceModelChanged()
{
//    qDebug() << "aaaaaaaaaaaaaa";
//    beginResetModel();

    // add all existing ones if they are not already in
    int appsCount = sourceModel()->rowCount();
    for (int i = 0; i < appsCount; i++) {
        QString desktopId(sourceModel()->data(sourceModel()->index(i, 0), AppItem::DesktopIdRole).toString());
        int folder, page, idx;
        std::tie(folder, std::ignore, std::ignore) = findItem(desktopId);
        if (folder == -1) {
//            qDebug() << desktopId;
            m_topLevel->appendItem(desktopId);
        }
    }

    // TODO: remove the ones that no longer valid out of m_folders

    saveItemArrangementToUserData();

//    endResetModel();
}

ItemsPage *MultipageProxyModel::createFolder(const QString &idNumber)
{
    QString fullId("internal/folders/" + idNumber);
    Q_ASSERT(!m_folders.contains(fullId));

//    int insertTo = rowCount(QModelIndex());
//    beginInsertRows(QModelIndex(), insertTo, insertTo);
    beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
    ItemsPage * page = new ItemsPage(4 * 3, this);
    m_folders.insert(fullId, page);
    endInsertRows();

    return page;
}

// get folder by id. 0 is top level, >=1 is folder
ItemsPage *MultipageProxyModel::folderById(int id)
{
    if (id == 0) return m_topLevel;
    const QString folderId("internal/folders/" + QString::number(id));
    return m_folders.value(folderId);
}
