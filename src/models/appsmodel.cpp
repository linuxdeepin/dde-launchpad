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
#include <appinfomonitor.h>

DCORE_USE_NAMESPACE

AppsModel::AppsModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_appInfoMonitor(new AppInfoMonitor(this))
    , m_dconfig(new DConfig("org.deepin.dde.launchpad.appsmodel"))
{
    Q_ASSERT_X(m_dconfig->isValid(), "DConfig", "DConfig file is missing or invalid");
    m_excludedAppIdList = m_dconfig->value("excludeAppIdList", QStringList{}).toStringList();

    QHash<int, QByteArray> defaultRoleNames = roleNames();
    defaultRoleNames.insert({
        {AppItem::DesktopIdRole, QByteArrayLiteral("desktopId")},
        {AppItem::DDECategoryRole, QByteArrayLiteral("category")},
        {AppItem::IconNameRole, QByteArrayLiteral("iconName")},
        {AppsModel::TransliteratedRole, QByteArrayLiteral("transliterated")}
    });
    setItemRoleNames(defaultRoleNames);

    QList<AppItem *> items(allAppInfosShouldBeShown());
    QList<AppItem *> duplicatedItems = addItems(items);
    Q_ASSERT(duplicatedItems.isEmpty());
    qDebug() << rowCount();

    connect(m_appInfoMonitor, &AppInfoMonitor::changed, this, [this](){
        qDebug() << "changed";
        // TODO release icon's cache when gtk's icon-theme.cache is updated.
        IconUtils::tryUpdateIconCache();
        QList<AppItem *> items(allAppInfosShouldBeShown());
        cleanUpInvalidApps(items);
        QList<AppItem *> duplicatedItems = updateItems(items);
        for (AppItem * item : qAsConst(duplicatedItems)) {
            delete item;
        }
    });
}

void AppsModel::appendRows(const QList<AppItem *> &items)
{
    // TODO: preformance improvement?
    for (AppItem * item : items) {
        appendRow(item);
    }
}

void AppsModel::insertRows(const QList<AppItem *> &items)
{
    // TODO: preformance improvement?
    for (AppItem * item : items) {
        insertRow(0, item);
    }
}

AppItem *AppsModel::itemFromDesktopId(const QString &freedesktopId)
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
    QList<AppItem *> inserts;
    QList<AppItem *> duplicated;

    for (AppItem * item : items) {
        AppItem * existing = itemFromDesktopId(item->freedesktopId());
        if (existing != nullptr) {
            existing->updateData(item);
            if (existing != item) {
                duplicated.append(item);
            }
        } else {
            inserts.insert(0, item);
        }
    }

    insertRows(inserts);
    return duplicated;
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case AppsModel::TransliteratedRole: {
        // TODO: 1. use icu::Transliterator for other locales
        //       2. support polyphonic characters (e.g. Music: YinYue or YinLe)
        const QString transliterated = Dtk::Core::pinyin(index.data(Qt::DisplayRole).toString(), Dtk::Core::TS_NoneTone).constFirst();
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

// the caller manage the return values' ownership (i.e. might need to free them)
QList<AppItem *> AppsModel::allAppInfosShouldBeShown() const
{
    const auto list = m_appInfoMonitor->allAppInfosShouldBeShown();
    QList<AppItem *> items;
    for (const QHash<QString, QString> & hashmap : list) {
        if (m_excludedAppIdList.contains(hashmap["id"])) {
            continue;
        }
        auto item = new AppItem(hashmap["id"]);
        item->setDisplayName(hashmap["name"]);
        item->setIconName(hashmap["icon"]);
        item->setCategories(hashmap["categories"].split(';'));
        item->setDDECategory(AppItem::DDECategories(CategoryUtils::parseBestMatchedCategory(hashmap["categories"].split(';'))));
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
