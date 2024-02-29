// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "categoryutils.h"
#include "iconutils.h"

#include <QDebug>
#include <DConfig>
#include <DPinyin>
#include <appinfo.h>
#include "appmgr.h"

DCORE_USE_NAMESPACE

static void updateAppItemFromAM(AppItem *appItem)
{
    const QString id(appItem->freedesktopId());
    auto item = AppMgr::instance()->appItem(id);
    if (!item) {
        qWarning() << "Not existing item in AppMgr for the desktopId" << id;
        return;
    }

    qDebug() << "update AppItem property for the desktopId" << id;
    appItem->setName(item->name);
    appItem->setDisplayName(item->displayName);
    appItem->setIconName(item->iconName);
    appItem->setCategories(item->categories);
    appItem->setInstalledTime(item->installedTime);
    appItem->setLastLaunchedTime(item->lastLaunchedTime);
    appItem->setLaunchedTimes(item->launchedTimes);
}

AppsModel::AppsModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_dconfig(DConfig::create("dde-launchpad", "org.deepin.dde.launchpad.appsmodel"))
{
    Q_ASSERT_X(m_dconfig->isValid(), "DConfig", "DConfig file is missing or invalid");
    m_excludedAppIdList = m_dconfig->value("excludeAppIdList", QStringList{}).toStringList();

    QHash<int, QByteArray> defaultRoleNames = roleNames();
    defaultRoleNames.insert({
        {AppItem::DesktopIdRole, QByteArrayLiteral("desktopId")},
        {AppItem::DDECategoryRole, QByteArrayLiteral("category")},
        {AppItem::IconNameRole, QByteArrayLiteral("iconName")},
        {AppItem::InstalledTimeRole, QByteArrayLiteral("installedTime")},
        {AppItem::LastLaunchedTimeRole, QByteArrayLiteral("lastLaunchedTime")},
        {AppItem::LaunchedTimesRole, QByteArrayLiteral("launchedTimes")},
        {AppsModel::TransliteratedRole, QByteArrayLiteral("transliterated")}
    });
    setItemRoleNames(defaultRoleNames);

    QList<AppItem *> items(allAppInfosShouldBeShown());
    QList<AppItem *> duplicatedItems = addItems(items);
    Q_ASSERT(duplicatedItems.isEmpty());
    qDebug() << rowCount();

    if (AppMgr::instance()->isValid()) {
        connect(AppMgr::instance(), &AppMgr::changed, this, &AppsModel::updateModelData);
        connect(AppMgr::instance(), &AppMgr::itemDataChanged, this, [this](const QString &id) {
            const auto appItem = this->appItem(id);
            if (!appItem) {
                qWarning() << "Not existing item in AppsModel for the desktopId" << id;
                return;
            }
            updateAppItemFromAM(appItem);
        });
    }
}

QList<AppItem *> AppsModel::appItems() const
{
    QList<AppItem *> items;
    for (int i = 0; i < rowCount(); i++) {
        if (auto appItem = dynamic_cast<AppItem*>(item(i))) {
            items.append(appItem);
        }
    }

    return items;
}

AppItem *AppsModel::appItem(const QString &desktopId) const
{
    const auto items = appItems();
    auto iter = std::find_if(items.begin(), items.end(), [desktopId](AppItem *item) {
        return item->freedesktopId() == desktopId;
    });
    return iter != items.end() ? *iter : nullptr;
}

void AppsModel::appendRows(const QList<AppItem *> items)
{
    // TODO: preformance improvement?
    for (AppItem * item : items) {
        appendRow(item);
    }
}

AppItem *AppsModel::itemFromDesktopId(const QString freedesktopId)
{
    QModelIndexList indexes = match(index(0, 0, QModelIndex()),
                                    AppItem::DesktopIdRole, freedesktopId, 1, Qt::MatchExactly);

    if (indexes.isEmpty()) return nullptr;

    return static_cast<AppItem *>(itemFromIndex(indexes.at(0)));
}

// the model takes the ownership for the items that actually added to the model.
// won't try to update item if there are existing ones.
// return the duplicated ones
const QList<AppItem *> AppsModel::addItems(const QList<AppItem *> &items)
{
    QList<AppItem *> append;
    QList<AppItem *> duplicated;

    for (AppItem * item : items) {
        if (itemFromDesktopId(item->freedesktopId()) != nullptr) {
            duplicated.append(item);
        } else {
            append.append(item);
        }
    }

    appendRows(append);
    return duplicated;
}

// try to update items, if not exist, add them to the model.
// the reference item that were used to update the existing one won't replace the existing one, thus
// they will be in the returned item list.
// return the ones were not added to the model.
const QList<AppItem *> AppsModel::updateItems(const QList<AppItem *> &items)
{
    QList<AppItem *> append;
    QList<AppItem *> duplicated;

    for (AppItem * item : items) {
        AppItem * existing = itemFromDesktopId(item->freedesktopId());
        if (existing != nullptr) {
            existing->updateData(item);
            if (existing != item) {
                duplicated.append(item);
            }
        } else {
            append.append(item);
        }
    }

    appendRows(append);
    return duplicated;
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case AppsModel::TransliteratedRole: {
        // TODO: 1. use icu::Transliterator for other locales
        //       2. support polyphonic characters (e.g. Music: YinYue or YinLe)
        const auto decodedDisplay = Dtk::Core::pinyin(index.data(Qt::DisplayRole).toString(), Dtk::Core::TS_NoneTone);
        if (decodedDisplay.isEmpty()) return QString();
        const QString transliterated = decodedDisplay.constFirst();
        if (transliterated.isEmpty()) return transliterated;
        const QChar & firstChar = transliterated.constData()[0];
        if (firstChar.isDigit()) return QString("#%1").arg(transliterated);
        else if (!firstChar.isLetter()) return QString("&%1").arg(transliterated);
        return transliterated;
    }
    default:
        break;
    }

    return QStandardItemModel::data(index, role);
}

void AppsModel::updateModelData()
{
    qDebug() << "changed";
    // TODO release icon's cache when gtk's icon-theme.cache is updated.
    IconUtils::tryUpdateIconCache();
    QList<AppItem *> items(allAppInfosShouldBeShown());
    cleanUpInvalidApps(items);
    QList<AppItem *> duplicatedItems = updateItems(items);
    for (AppItem * item : std::as_const(duplicatedItems)) {
        delete item;
    }
}

// the caller manage the return values' ownership (i.e. might need to free them)
QList<AppItem *> AppsModel::allAppInfosShouldBeShown() const
{
    QList<AppItem *> items;
    const auto list = AppMgr::instance()->allAppInfosShouldBeShown();
    for (auto appItem : list) {
        if (m_excludedAppIdList.contains(appItem->id)) {
            continue;
        }
        auto item = new AppItem(appItem->id);
        item->setName(appItem->name);
        item->setDisplayName(appItem->displayName);
        item->setIconName(appItem->iconName);
        item->setCategories(appItem->categories);
        item->setDDECategory(AppItem::DDECategories(CategoryUtils::parseBestMatchedCategory(appItem->categories)));
        item->setInstalledTime(appItem->installedTime);
        item->setLastLaunchedTime(appItem->lastLaunchedTime);
        item->setLaunchedTimes(appItem->launchedTimes);
        items.append(item);
    }
    return items;
}

// remove apps that are not in the \l knownExistedApps list
void AppsModel::cleanUpInvalidApps(const QList<AppItem *> knownExistedApps)
{
    QSet<QString> existedApps;
    for (const AppItem * app : knownExistedApps) {
        existedApps.insert(app->freedesktopId());
    }
    for (int i = rowCount() - 1; i >= 0; i--) {
        const QString & appId(data(index(i, 0), AppItem::DesktopIdRole).toString());
        if (!existedApps.contains(appId)) {
            removeRow(i);
        }
    }
}
