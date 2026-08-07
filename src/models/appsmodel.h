// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QAbstractListModel>
#include <QMetaObject>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QVariantMap>

namespace Dtk::Core {
class DFileWatcherManager;
}

class QTimer;

// Adapts dde-shell's application model to the roles and desktop IDs expected by launchpad.
class AppsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged FINAL)
    Q_PROPERTY(QVariantMap ddeCategories READ ddeCategories NOTIFY ddeCategoriesChanged FINAL)
    QML_NAMED_ELEMENT(AppsModel)
    QML_SINGLETON

public:
    enum Roles {
        DesktopIdRole = Qt::UserRole,
        CategoriesRole,
        DDECategoryRole,
        IconNameRole,
        NameRole,
        InstalledTimeRole,
        LastLaunchedTimeRole,
        LaunchedTimesRole,
        IsAutoStartRole,
        VendorRole,
        GenericNameRole,
        TransliteratedRole = 0x1000,
        AllTransliteratedRole,
        ProxyModelExtendedRole = 0x10000
    };
    Q_ENUM(Roles)

    static AppsModel &instance()
    {
        static AppsModel _instance;
        return _instance;
    }

    static AppsModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &instance();
    }

    void setSourceModel(QAbstractItemModel *sourceModel);
    QModelIndex indexFromDesktopId(const QString &desktopId) const;
    bool ready() const;
    QVariantMap ddeCategories() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void setReady(bool ready);
    void setDdeCategories(const QVariantMap &categories);

Q_SIGNALS:
    void readyChanged(bool ready);
    void ddeCategoriesChanged();

private:
    explicit AppsModel(QObject *parent = nullptr);

    int sourceRole(const QByteArray &name) const;
    QVariant sourceData(const QModelIndex &sourceIndex, const QByteArray &roleName) const;
    QList<int> mappedRoles(const QList<int> &sourceRoles) const;
    QString normalizedDesktopId(const QString &sourceId) const;
    int adapterRowForSourceRow(int sourceRow) const;
    void insertSourceRows(const QModelIndex &parent, int first, int last);
    void beginRemoveSourceRows(const QModelIndex &parent, int first, int last);
    void endRemoveSourceRows(const QModelIndex &parent, int first, int last);
    void updateSourceRows(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);
    void rebuildRows();
    bool acceptsSourceIndex(const QModelIndex &sourceIndex) const;
    bool shouldDelaySourceIndex(const QModelIndex &sourceIndex) const;
    void updateIconData();

    QStringList m_excludedAppIdList;
    Dtk::Core::DFileWatcherManager *m_iconCacheWatcher = nullptr;
    QTimer *m_iconUpdateTimer = nullptr;
    QPointer<QAbstractItemModel> m_sourceModel;
    QList<QPersistentModelIndex> m_rows;
    QList<QMetaObject::Connection> m_sourceConnections;
    QVariantMap m_ddeCategories;
    bool m_sourceRowsRemovalActive = false;
    bool m_ready = false;
};
