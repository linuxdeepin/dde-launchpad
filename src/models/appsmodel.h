// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appitem.h"

#include <QtQml/qqml.h>
#include <QStandardItemModel>
#include <QTimer>

namespace Dtk::Core {
class DConfig;
class DFileWatcherManager;
}

// List of applications and nothing else.
// Always in a single column so it's a one dimension model.
class AppsModel : public QStandardItemModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AppsModel)
    QML_SINGLETON
public:
    enum Roles {
        TransliteratedRole = AppItem::ModelExtendedRole,
        NameRole = AppItem::NameRole,
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

    void appendRows(const QList<AppItem *> items);

    AppItem * itemFromDesktopId(const QString freedesktopId);
    [[nodiscard("might need to free them")]] const QList<AppItem *> addItems(const QList<AppItem *> &items);
    [[nodiscard("might need to free them")]] const QList<AppItem *> updateItems(const QList<AppItem *> &items);

    // QAbstractItemModel interface
    QVariant data(const QModelIndex &index, int role) const override;

    AppItem *appItem(const QString &desktopId) const;
private slots:
    void updateModelData();

private:
    explicit AppsModel(QObject *parent = nullptr);

    QList<AppItem *> allAppInfosShouldBeShown() const;
    void cleanUpInvalidApps(const QList<AppItem *> knownExistedApps);
    QList<AppItem *> appItems() const;

    Dtk::Core::DConfig * m_dconfig;
    QStringList m_excludedAppIdList;
    Dtk::Core::DFileWatcherManager *m_fwIconCache = nullptr;
    QTimer *m_tmUpdateCache = nullptr;
};
