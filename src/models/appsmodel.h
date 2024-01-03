// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appitem.h"

#include <QStandardItemModel>

// List of applications and nothing else.
// Always in a single column so it's a one dimension model.
class AppInfoMonitor;
class AppsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum Roles {
        TransliteratedRole = AppItem::ModelExtendedRole,
        ProxyModelExtendedRole = 0x10000
    };

    static AppsModel &instance()
    {
        static AppsModel _instance;
        return _instance;
    }


    void appendRows(const QList<AppItem *> items);

    AppItem * itemFromDesktopId(const QString freedesktopId);
    [[nodiscard("might need to free them")]] const QList<AppItem *> addItems(const QList<AppItem *> &items);
    [[nodiscard("might need to free them")]] const QList<AppItem *> updateItems(const QList<AppItem *> &items);

    // QAbstractItemModel interface
    QVariant data(const QModelIndex &index, int role) const override;

private:
    explicit AppsModel(QObject *parent = nullptr);

    QList<AppItem *> allAppInfosShouldBeShown() const;
    void cleanUpInvalidApps(const QList<AppItem *> knownExistedApps);

    AppInfoMonitor * m_appInfoMonitor;

};
