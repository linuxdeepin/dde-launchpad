// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class FrequentlyUsedProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* recentlyInstalledModel READ recentlyInstalledModel WRITE setRecentlyInstalledModel NOTIFY recentlyInstalledModelChanged FINAL)
    QML_NAMED_ELEMENT(FrequentlyUsedProxyModel)
public:
    explicit FrequentlyUsedProxyModel(QObject *parent = nullptr);
    QAbstractItemModel *recentlyInstalledModel() const;
    void setRecentlyInstalledModel(QAbstractItemModel *newRecentlyInstalledModel);

signals:
    void recentlyInstalledModelChanged();

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
private:
    bool inRecentlyInstalledModel(const QModelIndex &index) const;
    bool lessThenByFrequentlyUsed(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const;

private:
    QPointer<QAbstractItemModel> m_recentlyInstalledModel;
    QStringList m_frequentlyUsedAppIdList;
};
