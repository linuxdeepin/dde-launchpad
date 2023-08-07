// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSortFilterProxyModel>

class CategorizedSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(CategoryType categoryType READ categoryType WRITE setCategoryType NOTIFY categoryTypeChanged)
    Q_PROPERTY(QString sortRoleName READ sortRoleName NOTIFY categoryTypeChanged)

public:
    static CategorizedSortProxyModel &instance()
    {
        static CategorizedSortProxyModel _instance;
        return _instance;
    }

    enum CategoryType {
        Alphabetary,
        DDECategory
    };
    Q_ENUM(CategoryType)

    void setCategoryType(enum CategoryType categoryType);
    enum CategoryType categoryType() const;

    QString sortRoleName() const;

signals:
    void categoryTypeChanged();

private:
    explicit CategorizedSortProxyModel(QObject *parent = nullptr);
};
