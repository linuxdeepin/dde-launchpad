// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSortFilterProxyModel>

class RecentlyInstalledProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    static RecentlyInstalledProxyModel &instance()
    {
        static RecentlyInstalledProxyModel _instance;
        return _instance;
    }

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    explicit RecentlyInstalledProxyModel(QObject *parent = nullptr);
};
