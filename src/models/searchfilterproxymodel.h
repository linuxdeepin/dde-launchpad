// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class SearchFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* recentlyInstalledModel READ recentlyInstalledModel WRITE setRecentlyInstalledModel NOTIFY recentlyInstalledModelChanged FINAL)
    QML_NAMED_ELEMENT(SearchFilterProxyModel)
    QML_SINGLETON
public:
    static SearchFilterProxyModel &instance()
    {
        static SearchFilterProxyModel _instance;
        return _instance;
    }

    static SearchFilterProxyModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &instance();
    }

    QAbstractItemModel *recentlyInstalledModel() const;
    void setRecentlyInstalledModel(QAbstractItemModel *newRecentlyInstalledModel);

signals:
    void recentlyInstalledModelChanged();

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
private:
    explicit SearchFilterProxyModel(QObject *parent = nullptr);
    bool inRecentlyInstalledModel(const QModelIndex &index) const;
    bool lessThenByFrequentlyUsed(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const;

private:
    QPointer<QAbstractItemModel> m_recentlyInstalledModel;
    QStringList m_frequentlyUsedAppIdList;
};
