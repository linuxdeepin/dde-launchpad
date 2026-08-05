// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"

#include "../ddeintegration/appmgr.h"
#include "iconutils.h"

#include <DConfig>
#include <DFileWatcherManager>
#include <DPinyin>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

Q_LOGGING_CATEGORY(logModels, "org.deepin.dde.launchpad.models")

DCORE_USE_NAMESPACE

namespace {
constexpr auto DesktopIdRoleName = "desktopId";
constexpr auto NameRoleName = "name";
constexpr auto IconNameRoleName = "iconName";
constexpr auto NoDisplayRoleName = "noDisplay";
constexpr auto DDECategoryRoleName = "ddeCategory";
constexpr auto CategoriesRoleName = "categories";
constexpr auto InstalledTimeRoleName = "installedTime";
constexpr auto LastLaunchedTimeRoleName = "lastLaunchedTime";
constexpr auto LaunchedTimesRoleName = "launchedTimes";
constexpr auto AutoStartRoleName = "autoStart";
constexpr auto VendorRoleName = "vendor";
constexpr auto GenericNameRoleName = "genericName";
}

AppsModel::AppsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_iconCacheWatcher(new DFileWatcherManager(this))
    , m_iconUpdateTimer(new QTimer(this))
{
    qCDebug(logModels) << "Initializing shared AppsModel adapter";
    const auto config = DConfig::create("org.deepin.dde.shell", "org.deepin.ds.launchpad", QString(), this);
    Q_ASSERT_X(config->isValid(), "DConfig", "DConfig file is missing or invalid");
    m_excludedAppIdList = config->value("excludeAppIdList", QStringList {}).toStringList();
    m_iconUpdateTimer->setInterval(1000);
    m_iconUpdateTimer->setSingleShot(true);

    const QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    const QString suffix = QStringLiteral("/icons/hicolor/icon-theme.cache");
    for (const QString &path : paths) {
        if (QFileInfo::exists(path + suffix))
            m_iconCacheWatcher->add(path + suffix);
    }

    connect(m_iconCacheWatcher, &DFileWatcherManager::fileModified, m_iconUpdateTimer, qOverload<>(&QTimer::start));
    connect(m_iconCacheWatcher, &DFileWatcherManager::fileAttributeChanged, m_iconUpdateTimer, qOverload<>(&QTimer::start));
    connect(m_iconUpdateTimer, &QTimer::timeout, this, &AppsModel::updateIconData);
    connect(AppMgr::instance(), &AppMgr::pendingAppItemReady, this, [this](const QString &desktopId) {
        if (!m_sourceModel)
            return;

        for (int sourceRow = 0; sourceRow < m_sourceModel->rowCount(); ++sourceRow) {
            const QModelIndex sourceIndex = m_sourceModel->index(sourceRow, 0);
            if (normalizedDesktopId(sourceData(sourceIndex, DesktopIdRoleName).toString()) != desktopId)
                continue;
            if (!acceptsSourceIndex(sourceIndex))
                return;

            const QPersistentModelIndex persistentSourceIndex(sourceIndex);
            if (m_rows.contains(persistentSourceIndex))
                return;

            const int adapterRow = adapterRowForSourceRow(sourceRow);
            beginInsertRows({}, adapterRow, adapterRow);
            m_rows.insert(adapterRow, persistentSourceIndex);
            endInsertRows();
            return;
        }
    });
}

void AppsModel::setSourceModel(QAbstractItemModel *model)
{
    if (m_sourceModel == model && (model || m_rows.isEmpty()))
        return;

    for (const auto &connection : std::as_const(m_sourceConnections))
        QObject::disconnect(connection);
    m_sourceConnections.clear();
    AppMgr::instance()->clearPendingAppItems();

    setReady(false);
    beginResetModel();
    m_sourceModel = model;
    m_rows.clear();
    endResetModel();

    if (!model)
        return;

    m_sourceConnections << connect(model, &QAbstractItemModel::dataChanged, this, &AppsModel::updateSourceRows);
    m_sourceConnections << connect(model, &QAbstractItemModel::rowsInserted, this, &AppsModel::insertSourceRows);
    m_sourceConnections << connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &AppsModel::beginRemoveSourceRows);
    m_sourceConnections << connect(model, &QAbstractItemModel::rowsRemoved, this, &AppsModel::endRemoveSourceRows);
    m_sourceConnections << connect(model, &QAbstractItemModel::rowsMoved, this, &AppsModel::rebuildRows);
    m_sourceConnections << connect(model, &QAbstractItemModel::modelReset, this, &AppsModel::rebuildRows);
    m_sourceConnections << connect(model, &QAbstractItemModel::layoutChanged, this, &AppsModel::rebuildRows);
    m_sourceConnections << connect(model, &QObject::destroyed, this, [this](QObject *) {
        setSourceModel(nullptr);
    });
    rebuildRows();
}

QModelIndex AppsModel::indexFromDesktopId(const QString &desktopId) const
{
    if (rowCount() == 0)
        return {};
    const auto matches = match(index(0, 0), AppsModel::DesktopIdRole,
                               normalizedDesktopId(desktopId), 1, Qt::MatchExactly);
    return matches.value(0);
}

bool AppsModel::ready() const
{
    return m_ready;
}

QVariantMap AppsModel::ddeCategories() const
{
    return m_ddeCategories;
}

int AppsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

void AppsModel::setReady(bool ready)
{
    if (m_ready == ready)
        return;
    m_ready = ready;
    Q_EMIT readyChanged(ready);
}

void AppsModel::setDdeCategories(const QVariantMap &categories)
{
    if (m_ddeCategories == categories)
        return;
    m_ddeCategories = categories;
    Q_EMIT ddeCategoriesChanged();
}

QVariant AppsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (index.row() < 0 || index.row() >= m_rows.size())
        return {};
    const QModelIndex sourceIndex = m_rows.at(index.row());
    if (!sourceIndex.isValid())
        return {};

    switch (role) {
    case Qt::DisplayRole:
    case AppsModel::NameRole:
        return sourceData(sourceIndex, NameRoleName);
    case AppsModel::DesktopIdRole:
        return normalizedDesktopId(sourceData(sourceIndex, DesktopIdRoleName).toString());
    case AppsModel::CategoriesRole:
        return sourceData(sourceIndex, CategoriesRoleName);
    case AppsModel::DDECategoryRole:
        return sourceData(sourceIndex, DDECategoryRoleName);
    case AppsModel::IconNameRole: {
        const QString iconName = sourceData(sourceIndex, IconNameRoleName).toString();
        if (iconName.isEmpty())
            return QStringLiteral("application-x-desktop");
        return QFileInfo(iconName).isAbsolute() ? QUrl::fromLocalFile(iconName).toString() : iconName;
    }
    case AppsModel::InstalledTimeRole:
        return sourceData(sourceIndex, InstalledTimeRoleName);
    case AppsModel::LastLaunchedTimeRole:
        return sourceData(sourceIndex, LastLaunchedTimeRoleName);
    case AppsModel::LaunchedTimesRole:
        return sourceData(sourceIndex, LaunchedTimesRoleName);
    case AppsModel::IsAutoStartRole:
        return sourceData(sourceIndex, AutoStartRoleName);
    case AppsModel::VendorRole:
        return sourceData(sourceIndex, VendorRoleName);
    case AppsModel::GenericNameRole:
        return sourceData(sourceIndex, GenericNameRoleName);
    case AppsModel::TransliteratedRole: {
        const auto decodedDisplay = Dtk::Core::pinyin(data(index, Qt::DisplayRole).toString(), Dtk::Core::TS_NoneTone);
        if (decodedDisplay.isEmpty())
            return QString();
        const QString transliterated = decodedDisplay.constFirst();
        if (transliterated.isEmpty())
            return transliterated;
        const QChar firstChar = transliterated.at(0);
        if (firstChar.isDigit())
            return QStringLiteral("#") + transliterated;
        if (!firstChar.isLetter())
            return QStringLiteral("&") + transliterated;
        return transliterated;
    }
    case AppsModel::AllTransliteratedRole:
        return Dtk::Core::pinyin(data(index, Qt::DisplayRole).toString(), Dtk::Core::TS_NoneTone).join('.');
    default:
        return {};
    }
}

QHash<int, QByteArray> AppsModel::roleNames() const
{
    return {
        { Qt::DisplayRole, QByteArrayLiteral("display") },
        { AppsModel::DesktopIdRole, QByteArrayLiteral("desktopId") },
        { AppsModel::CategoriesRole, QByteArrayLiteral("categories") },
        { AppsModel::DDECategoryRole, QByteArrayLiteral("category") },
        { AppsModel::IconNameRole, QByteArrayLiteral("iconName") },
        { AppsModel::NameRole, QByteArrayLiteral("name") },
        { AppsModel::InstalledTimeRole, QByteArrayLiteral("installedTime") },
        { AppsModel::LastLaunchedTimeRole, QByteArrayLiteral("lastLaunchedTime") },
        { AppsModel::LaunchedTimesRole, QByteArrayLiteral("launchedTimes") },
        { AppsModel::IsAutoStartRole, QByteArrayLiteral("autoStart") },
        { AppsModel::VendorRole, QByteArrayLiteral("vendor") },
        { AppsModel::GenericNameRole, QByteArrayLiteral("genericName") },
        { AppsModel::TransliteratedRole, QByteArrayLiteral("transliterated") },
        { AppsModel::AllTransliteratedRole, QByteArrayLiteral("allTransliterated") },
    };
}

bool AppsModel::acceptsSourceIndex(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return false;

    const QString desktopId = normalizedDesktopId(sourceData(sourceIndex, DesktopIdRoleName).toString());
    if (desktopId.isEmpty() || m_excludedAppIdList.contains(desktopId))
        return false;

    return !sourceData(sourceIndex, NoDisplayRoleName).toBool();
}

int AppsModel::sourceRole(const QByteArray &name) const
{
    return m_sourceModel ? m_sourceModel->roleNames().key(name, -1) : -1;
}

QVariant AppsModel::sourceData(const QModelIndex &sourceIndex, const QByteArray &roleName) const
{
    const int role = sourceRole(roleName);
    return role >= 0 && m_sourceModel ? m_sourceModel->data(sourceIndex, role) : QVariant();
}

QList<int> AppsModel::mappedRoles(const QList<int> &sourceRoles) const
{
    if (sourceRoles.isEmpty())
        return {};

    QList<int> result;
    const auto appendIfChanged = [this, &sourceRoles, &result](int targetRole, const QByteArray &sourceName) {
        if (sourceRoles.contains(sourceRole(sourceName)))
            result.append(targetRole);
    };

    appendIfChanged(AppsModel::DesktopIdRole, DesktopIdRoleName);
    appendIfChanged(AppsModel::CategoriesRole, CategoriesRoleName);
    appendIfChanged(AppsModel::DDECategoryRole, DDECategoryRoleName);
    appendIfChanged(AppsModel::IconNameRole, IconNameRoleName);
    appendIfChanged(AppsModel::InstalledTimeRole, InstalledTimeRoleName);
    appendIfChanged(AppsModel::LastLaunchedTimeRole, LastLaunchedTimeRoleName);
    appendIfChanged(AppsModel::LaunchedTimesRole, LaunchedTimesRoleName);
    appendIfChanged(AppsModel::IsAutoStartRole, AutoStartRoleName);
    appendIfChanged(AppsModel::VendorRole, VendorRoleName);
    appendIfChanged(AppsModel::GenericNameRole, GenericNameRoleName);

    if (sourceRoles.contains(sourceRole(NameRoleName))) {
        result << Qt::DisplayRole << AppsModel::NameRole << AppsModel::TransliteratedRole << AppsModel::AllTransliteratedRole;
    }
    QList<int> uniqueRoles;
    for (int role : std::as_const(result)) {
        if (!uniqueRoles.contains(role))
            uniqueRoles.append(role);
    }
    return uniqueRoles;
}

QString AppsModel::normalizedDesktopId(const QString &sourceId) const
{
    if (sourceId.isEmpty() || sourceId.endsWith(QLatin1String(".desktop")) || sourceId.startsWith(QLatin1String("internal/")))
        return sourceId;
    return sourceId + QStringLiteral(".desktop");
}

int AppsModel::adapterRowForSourceRow(int sourceRow) const
{
    int row = 0;
    while (row < m_rows.size() && m_rows.at(row).row() < sourceRow)
        ++row;
    return row;
}

void AppsModel::insertSourceRows(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid() || !m_sourceModel)
        return;

    QList<QPersistentModelIndex> insertedRows;
    for (int sourceRow = first; sourceRow <= last; ++sourceRow) {
        const QModelIndex sourceIndex = m_sourceModel->index(sourceRow, 0);
        if (acceptsSourceIndex(sourceIndex) && !shouldDelaySourceIndex(sourceIndex))
            insertedRows.append(sourceIndex);
    }
    if (insertedRows.isEmpty())
        return;

    const int adapterRow = adapterRowForSourceRow(first);
    beginInsertRows({}, adapterRow, adapterRow + insertedRows.size() - 1);
    for (int offset = 0; offset < insertedRows.size(); ++offset)
        m_rows.insert(adapterRow + offset, insertedRows.at(offset));
    endInsertRows();
}

void AppsModel::beginRemoveSourceRows(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    for (int sourceRow = first; sourceRow <= last; ++sourceRow) {
        const QModelIndex sourceIndex = m_sourceModel->index(sourceRow, 0);
        AppMgr::instance()->cancelPendingAppItem(
                normalizedDesktopId(sourceData(sourceIndex, DesktopIdRoleName).toString()));
    }

    const int adapterFirst = adapterRowForSourceRow(first);
    int count = 0;
    while (adapterFirst + count < m_rows.size() && m_rows.at(adapterFirst + count).row() <= last)
        ++count;
    if (count == 0)
        return;

    beginRemoveRows({}, adapterFirst, adapterFirst + count - 1);
    m_rows.remove(adapterFirst, count);
    m_sourceRowsRemovalActive = true;
}

void AppsModel::endRemoveSourceRows(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    if (!m_sourceRowsRemovalActive)
        return;

    m_sourceRowsRemovalActive = false;
    endRemoveRows();
}

void AppsModel::updateSourceRows(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    if (topLeft.parent().isValid() || bottomRight.parent().isValid() || !m_sourceModel)
        return;

    const int desktopIdRole = sourceRole(DesktopIdRoleName);
    const int noDisplayRole = sourceRole(NoDisplayRoleName);
    const bool mayChangeMembership = roles.isEmpty() || roles.contains(desktopIdRole) || roles.contains(noDisplayRole);
    const QList<int> targetRoles = mappedRoles(roles);

    for (int sourceRow = topLeft.row(); sourceRow <= bottomRight.row(); ++sourceRow) {
        const QModelIndex sourceIndex = m_sourceModel->index(sourceRow, 0);
        const QPersistentModelIndex persistentSourceIndex(sourceIndex);
        const int currentRow = m_rows.indexOf(persistentSourceIndex);
        const bool accepted = acceptsSourceIndex(sourceIndex);

        const QString desktopId = normalizedDesktopId(sourceData(sourceIndex, DesktopIdRoleName).toString());
        const bool pending = AppMgr::instance()->isPendingAppItem(desktopId);
        if ((mayChangeMembership || pending) && accepted && currentRow < 0) {
            if (shouldDelaySourceIndex(sourceIndex))
                continue;
            const int adapterRow = adapterRowForSourceRow(sourceRow);
            beginInsertRows({}, adapterRow, adapterRow);
            m_rows.insert(adapterRow, persistentSourceIndex);
            endInsertRows();
            continue;
        }
        if (mayChangeMembership && !accepted && currentRow >= 0) {
            beginRemoveRows({}, currentRow, currentRow);
            m_rows.removeAt(currentRow);
            endRemoveRows();
            continue;
        }
        if (accepted && currentRow >= 0 && (roles.isEmpty() || !targetRoles.isEmpty()))
            Q_EMIT dataChanged(index(currentRow, 0), index(currentRow, 0), targetRoles);
    }
}

void AppsModel::rebuildRows()
{
    beginResetModel();
    m_rows.clear();
    if (m_sourceModel) {
        for (int row = 0; row < m_sourceModel->rowCount(); ++row) {
            const QModelIndex sourceIndex = m_sourceModel->index(row, 0);
            const QString desktopId = normalizedDesktopId(sourceData(sourceIndex, DesktopIdRoleName).toString());
            if (acceptsSourceIndex(sourceIndex) && !AppMgr::instance()->isPendingAppItem(desktopId))
                m_rows.append(sourceIndex);
        }
    }
    endResetModel();
}

bool AppsModel::shouldDelaySourceIndex(const QModelIndex &sourceIndex) const
{
    const QString desktopId = normalizedDesktopId(sourceData(sourceIndex, DesktopIdRoleName).toString());
    const QString iconName = sourceData(sourceIndex, IconNameRoleName).toString();
    return AppMgr::instance()->waitForIcon(desktopId, iconName);
}

void AppsModel::updateIconData()
{
    IconUtils::tryUpdateIconCache();
    if (rowCount() > 0)
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), { AppsModel::IconNameRole });
}
