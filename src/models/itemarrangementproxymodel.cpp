// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemarrangementproxymodel.h"

#include "appsmodel.h"
#include "categoryutils.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

ItemArrangementProxyModel::~ItemArrangementProxyModel()
{

}

int ItemArrangementProxyModel::pageCount(int folderId) const
{
    if (folderId == 0) return m_topLevel->pageCount();

    QString fullId("internal/folders/" + QString::number(folderId));
    Q_ASSERT(m_folders.contains(fullId));

    if (auto itemPage = m_folders.value(fullId); itemPage) {
        return itemPage->pageCount();
    } else {
        qWarning() << "itemPage is null, return 0. fullId is" << fullId;
        return 0;
    }
}

void ItemArrangementProxyModel::updateFolderName(int folderId, const QString &name)
{
    ItemsPage * folder = folderById(folderId);
    folder->setName(name);

    QModelIndexList matched = match(mapFromSource(m_folderModel.index(0, 0)), AppItem::DesktopIdRole, QString("internal/folders/%1").arg(folderId));
    Q_ASSERT(!matched.isEmpty());
    emit dataChanged(matched.constFirst(), matched.constFirst(), { Qt::DisplayRole });

    saveItemArrangementToUserData();
}

void ItemArrangementProxyModel::bringToFront(const QString & id)
{
    std::tuple<int, int, int> origPos = findItem(id);

    // can only bring top-level item to front
    if (std::get<0>(origPos) != 0) return;

    // already at front
    if (std::get<1>(origPos) == 0 && std::get<2>(origPos) == 0) return;

    m_topLevel->moveItemPosition(std::get<1>(origPos), std::get<2>(origPos), 0, 0, false);

    saveItemArrangementToUserData();

    // Lazy solution, just notify the view that all rows and its roles are changed so they need to be updated.
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {
        PageRole, IndexInPageRole, FolderIdNumberRole, IconsNameRole
    });
}

void ItemArrangementProxyModel::commitDndOperation(const QString &dragId, const QString &dropId, const DndOperation op, int pageHint)
{
    if (dragId == dropId) return;

    std::tuple<int, int, int> dragOrigPos = findItem(dragId);
    std::tuple<int, int, int> dropOrigPos = findItem(dropId);


    Q_ASSERT(std::get<0>(dragOrigPos) != -1);
    if (std::get<0>(dragOrigPos) == -1) {
        qWarning() << "Cannot found" << dragId << "in current item arrangement.";
        return;
    }

    if (op != DndOperation::DndJoin) {
        // move to dropId's front or back
        if (std::get<0>(dragOrigPos) == std::get<0>(dropOrigPos)) {
            // same folder item re-arrangement
            ItemsPage * folder = folderById(std::get<0>(dropOrigPos));
            const int dragOrigPage = std::get<1>(dragOrigPos);
            const int dropOrigPage = std::get<1>(dropOrigPos);
            const int fromIndex = std::get<2>(dragOrigPos);
            const int toIndex = std::get<2>(dropOrigPos);
            qDebug() << "dragOrigPage" << dragOrigPage << "dropOrigPage" << dropOrigPage << "fromIndex" << fromIndex << "toIndex" << toIndex << "op" << (op == DndOperation::DndAppend);
            folder->moveItemPosition(dragOrigPage, fromIndex, dropOrigPage, toIndex, op == DndOperation::DndAppend);
        } else {
            // different folder item arrangement
            ItemsPage * srcFolder = folderById(std::get<0>(dragOrigPos));
            ItemsPage * dstFolder = folderById(std::get<0>(dropOrigPos));
            srcFolder->removeItem(dragId);
            if (srcFolder->pageCount() == 0 && srcFolder != dstFolder) {
                removeFolder(QString::number(std::get<0>(dragOrigPos)));
            }
            dstFolder->insertItem(dragId, std::get<1>(dropOrigPos), std::get<2>(dropOrigPos));
        }
    } else {
        if (dragId.startsWith("internal/folders/") && dropId != "internal/folders/0") return; // cannot drag folder onto something
        if (std::get<0>(dropOrigPos) != 0 && dropId != "internal/folders/0") return; // folder inside folder is not allowed

        // the source item will be inside a new folder anyway.
        const int srcFolderId = std::get<0>(dragOrigPos);
        ItemsPage * srcFolder = folderById(srcFolderId);

        if (dropId.startsWith("internal/folders/")) {
            // drop into existing folder
            const int dropOrigFolder = QStringView{dropId}.mid(17).toInt();
            ItemsPage * dstFolder = folderById(dropOrigFolder);
            const int fromPage = std::get<1>(dragOrigPos);
            const int &toPage = pageHint;
            if (srcFolder == dstFolder)
                if (srcFolder->itemCount() == 1 ||
                    (fromPage == toPage && srcFolder->itemCount(fromPage) == 1))// dnd the only item to the same page
                    return;

            // hold the empty page avoid access out of page range !
            srcFolder->removeItem(dragId, false);
            if (srcFolder->itemCount() == 0 && srcFolder != dstFolder) {
                removeFolder(QString::number(srcFolderId));
            }
            dstFolder->insertItemToPage(dragId, pageHint);

            // clear empty page
            srcFolder->removeEmptyPages();
        } else {
            srcFolder->removeItem(dragId);
            // make a new folder, move two items into the folder
            QString dstFolderId = findAvailableFolderId();
            ItemsPage * dstFolder = createFolder(dstFolderId);
            dstFolder->appendPage({dropId, dragId});
            AppItem * dropItem = AppsModel::instance().itemFromDesktopId(dropId);
            AppItem::DDECategories dropCategories = AppItem::DDECategories(CategoryUtils::parseBestMatchedCategory(dropItem->categories()));
            dstFolder->setName("internal/category/" + QString::number(dropCategories));
            if (srcFolder->pageCount() == 0 && srcFolder != m_topLevel) {
                removeFolder(QString::number(srcFolderId));
            }
            m_topLevel->insertItem(dstFolderId, std::get<1>(dropOrigPos), std::get<2>(dropOrigPos));
            m_topLevel->removeItem(dropId);
        }
    }

    saveItemArrangementToUserData();

    // Lazy solution, just notify the view that all rows and its roles are changed so they need to be updated.
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {
        PageRole, IndexInPageRole, FolderIdNumberRole, IconsNameRole
    });
}

// return new empty page index
int ItemArrangementProxyModel::creatEmptyPage(int folderId) const
{
    if(folderId == 0){
        m_topLevel->appendEmptyPage();
        return m_topLevel->pageCount() - 1;
    }
    QString fullId("internal/folders/" + QString::number(folderId));
    Q_ASSERT(m_folders.contains(fullId));

    if (auto itemPage = m_folders.value(fullId); itemPage) {
        itemPage->appendEmptyPage();
        return itemPage->pageCount() - 1;
    } else {
        qWarning() << "itemPage create empty page false, return 0. fullId is" << fullId;
        return 0;
    }
}

void ItemArrangementProxyModel::removeEmptyPage() const
{
    m_topLevel->removeEmptyPages();
}

QVariant ItemArrangementProxyModel::data(const QModelIndex &index, int role) const
{
    int idx = index.row() - AppsModel::instance().rowCount();
    if (idx < 0 && role < AppsModel::ProxyModelExtendedRole) return QConcatenateTablesProxyModel::data(index, role);

    if (idx < 0) {
        // regular applications, not a folder
        QString id(data(index, AppItem::DesktopIdRole).toString());
        if (id.isEmpty() || id.contains("internal")) {
            qDebug() << id << index << index.row() << AppsModel::instance().rowCount() << role;
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
            case ItemTypeRole:
                return AppItemType;
        }
    } else {
        // a folder
        QModelIndex srcIdx = mapToSource(index);
        QString id = m_folderModel.itemFromIndex(srcIdx)->data(AppItem::DesktopIdRole).toString();
        int folder, page, pos;
        if (role >= AppsModel::ProxyModelExtendedRole && role != IconsNameRole) {
            std::tie(folder, page, pos) = findItem(id, true);
        }

        switch (role) {
            case Qt::DisplayRole:
                return m_folders.value(id)->name();
            case AppItem::DesktopIdRole:
                return id;
            case AppItem::IsAutoStartRole:
                return false;
            case PageRole:
                return page;
            case IndexInPageRole:
                return pos;
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
            case ItemTypeRole:
                return FolderItemType;
        }
    }
    return QConcatenateTablesProxyModel::data(index, role);
}

QHash<int, QByteArray> ItemArrangementProxyModel::roleNames() const
{
    auto existingRoleNames = AppsModel::instance().roleNames();
    existingRoleNames.insert(IconsNameRole, QByteArrayLiteral("folderIcons"));
    existingRoleNames.insert(ItemTypeRole, QByteArrayLiteral("itemType"));
    return existingRoleNames;
}

ItemArrangementProxyModel::ItemArrangementProxyModel(QObject *parent)
    : QConcatenateTablesProxyModel(parent)
    , m_topLevel(new ItemsPage(8 * 4, this))
{
    m_folderModel.setItemRoleNames(AppsModel::instance().roleNames());

    loadItemArrangementFromUserData();
    addSourceModel(&AppsModel::instance());

    onSourceModelChanged();
    onFolderModelChanged();

    connect(&AppsModel::instance(), &AppsModel::rowsInserted, this, &ItemArrangementProxyModel::onSourceModelChanged);
    connect(&AppsModel::instance(), &AppsModel::rowsRemoved, this, &ItemArrangementProxyModel::onSourceModelChanged);

    connect(&m_folderModel, &QStandardItemModel::rowsInserted, this, &ItemArrangementProxyModel::onFolderModelChanged);
    connect(&m_folderModel, &QStandardItemModel::rowsRemoved, this, &ItemArrangementProxyModel::onFolderModelChanged);

    connect(m_topLevel, &ItemsPage::pageCountChanged, this, &ItemArrangementProxyModel::topLevelPageCountChanged);
}

void ItemArrangementProxyModel::loadItemArrangementFromUserData()
{
    const QString arrangementSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
    const QString arrangementSettingPath(QDir(arrangementSettingBasePath).absoluteFilePath("deepin/dde-launchpad/item-arrangement.ini"));
    QSettings itemArrangementSettings(arrangementSettingPath, QSettings::NativeFormat);

    itemArrangementSettings.beginGroup("fullscreen");
    const QStringList folderGroups(itemArrangementSettings.childGroups());

    for (const QString & groupName : folderGroups) {
        itemArrangementSettings.beginGroup(groupName);
        QString folderName = itemArrangementSettings.value("name", QString()).toString();
        int pageCount = itemArrangementSettings.value("pageCount", 0).toInt();
        bool isTopLevel = groupName == "toplevel";

        qDebug() << groupName << folderName << pageCount;

        ItemsPage * page = isTopLevel ? m_topLevel : createFolder(groupName);
        page->setName(folderName);

        for (int i = 0; i < pageCount; i++) {
            QStringList items = itemArrangementSettings.value(QString::asprintf("pageItems/%d", i)).toStringList();
            page->appendPage(items);
        }

        itemArrangementSettings.endGroup();
    }
}

void ItemArrangementProxyModel::saveItemArrangementToUserData()
{
    const QString arrangementSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
    const QString arrangementSettingPath(QDir(arrangementSettingBasePath).absoluteFilePath("deepin/dde-launchpad/item-arrangement.ini"));
    QSettings itemArrangementSettings(arrangementSettingPath, QSettings::NativeFormat);
    itemArrangementSettings.clear();

    itemArrangementSettings.beginGroup("fullscreen/toplevel");
    int pageCount = m_topLevel->pageCount();
    itemArrangementSettings.setValue("pageCount", pageCount);
    for (int i = 0; i < pageCount; i++) {
        itemArrangementSettings.setValue(QString::asprintf("pageItems/%d", i), m_topLevel->items(i));
    }
    itemArrangementSettings.endGroup();

    for (int i = 0; i < m_folderModel.rowCount(); i++) {
        const QString & id = m_folderModel.index(i, 0).data(AppItem::DesktopIdRole).toString();
        itemArrangementSettings.beginGroup("fullscreen/" + id.mid(17));
        ItemsPage * page = m_folders.value(id);
        int pageCount = page->pageCount();
        itemArrangementSettings.setValue("name", page->name());
        itemArrangementSettings.setValue("pageCount", pageCount);
        for (int j = 0; j < pageCount; j++) {
            itemArrangementSettings.setValue(QString::asprintf("pageItems/%d", j), page->items(j));
        }
        itemArrangementSettings.endGroup();
    }

    itemArrangementSettings.sync();
}

std::tuple<int, int, int> ItemArrangementProxyModel::findItem(const QString &id, bool searchTopLevelOnly) const
{
    int page, idx;

    std::tie(page, idx) = m_topLevel->findItem(id);
    if (page != -1) return std::make_tuple(0, page, idx);

    if (!searchTopLevelOnly) {
        for (int i = 0; i < m_folderModel.rowCount(); i++) {
            const QString & folderId = m_folderModel.index(i, 0).data(AppItem::DesktopIdRole).toString();
            std::tie(page, idx) = m_folders[folderId]->findItem(id);
            if (page != -1) {
                return std::make_tuple(QStringView{folderId}.mid(17).toInt(), page, idx);
            }
        }
    }

    return std::make_tuple(-1, -1, -1);
}

void ItemArrangementProxyModel::onSourceModelChanged()
{
    QSet<QString> appDesktopIdSet;
    int appsCount = AppsModel::instance().rowCount();
    for (int i = 0; i < appsCount; i++) {
        QString desktopId(AppsModel::instance().data(AppsModel::instance().index(i, 0), AppItem::DesktopIdRole).toString());
        appDesktopIdSet.insert(desktopId);
        int folder;
        std::tie(folder, std::ignore, std::ignore) = findItem(desktopId);
        // add all existing ones if they are not already in
        if (folder == -1) {
            findItem(desktopId);
            // Find first available page with space
            int targetPage = -1;
            int targetIndex = -1;
            
            // Check each page for available space
            for (int page = 0; page < m_topLevel->pageCount(); page++) {
                int itemCount = m_topLevel->itemCount(page);
                if (itemCount < m_topLevel->maxItemCountPerPage()) {
                    targetPage = page;
                    targetIndex = itemCount;
                    break;
                }
            }
            
            if (targetPage != -1) {
                // Add to first available page with space
                m_topLevel->insertItem(desktopId, targetPage, targetIndex);
            } else {
                // All pages are full, append to new page
                m_topLevel->appendItem(desktopId);
            }
        }
    }

    m_topLevel->removeItemsNotIn(appDesktopIdSet);
    for (int i = m_folderModel.rowCount() - 1; i >= 0 ; i--) {
        const QString & folderId = m_folderModel.index(i, 0).data(AppItem::DesktopIdRole).toString();
        m_folders.value(folderId)->removeItemsNotIn(appDesktopIdSet);
        if (m_folders.value(folderId)->pageCount() == 0) {
            removeFolder(QString(folderId).remove("internal/folders/"));
        }
    }

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {
        PageRole, IndexInPageRole, FolderIdNumberRole, IconsNameRole
    });

    saveItemArrangementToUserData();
}

void ItemArrangementProxyModel::onFolderModelChanged()
{
    // if the QStandardItemModel is empty, adding the empty model to QConcatenateTablesProxyModel will result
    // the complete model is ill-formed (why?). Thus we only add them to the QConcatenateTablesProxyModel when
    // m_folderModel is not empty.
    // If m_folerModel is back to empty, we don't need to remove it from the model, and if we do that, it will
    // also result a crash (why?).
    if (m_folderModel.rowCount() != 0 && !sourceModels().contains(&m_folderModel)) {
        addSourceModel(&m_folderModel);
    }
}

QString ItemArrangementProxyModel::findAvailableFolderId()
{
    int idNumber = 0;
    QString fullId;
    do {
        idNumber++;
        fullId = QStringLiteral("internal/folders/%1").arg(idNumber);
    } while (m_folders.contains(fullId));

    Q_ASSERT(idNumber != 0); // 0 is reserved for top level.
    return fullId;
}

ItemsPage *ItemArrangementProxyModel::createFolder(const QString &id)
{
    Q_ASSERT(!id.isEmpty());
    QString fullId(id.startsWith("internal/folders/") ? id : QStringLiteral("internal/folders/%1").arg(id));
    Q_ASSERT(m_folderModel.findItems(fullId).isEmpty());

    ItemsPage * page = new ItemsPage(4 * 3, this);
    m_folders.insert(fullId, page);
    QStandardItem * folder = new QStandardItem(fullId);
    folder->setData(fullId, AppItem::DesktopIdRole);
    m_folderModel.appendRow(folder);

    connect(page, &ItemsPage::pageCountChanged, this, [this, fullId]() {
        int folderId = QStringView{fullId}.mid(17).toInt();
        emit folderPageCountChanged(folderId);
    });

    return page;
}

void ItemArrangementProxyModel::removeFolder(const QString &idNumber)
{
    QString fullId("internal/folders/" + idNumber);
    Q_ASSERT(m_folders.contains(fullId));

    m_folders.remove(fullId);
    m_topLevel->removeItem(fullId);
    QList<QStandardItem*> result = m_folderModel.findItems(fullId);
    m_folderModel.removeRows(result.first()->row(), 1);
}

// get folder by id. 0 is top level, >=1 is folder
ItemsPage *ItemArrangementProxyModel::folderById(int id)
{
    if (id == 0) return m_topLevel;
    const QString folderId("internal/folders/" + QString::number(id));
    return m_folders.value(folderId);
}

QStringList ItemArrangementProxyModel::allArrangedItems() const
{
    return m_topLevel->allArrangedItems();
}
