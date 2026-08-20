// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class FrequentlyUsedProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QAbstractItemModel* recentlyInstalledModel READ recentlyInstalledModel WRITE setRecentlyInstalledModel NOTIFY recentlyInstalledModelChanged FINAL)
    Q_PROPERTY(int desktopIdRole MEMBER m_desktopIdRole NOTIFY desktopIdRoleChanged)
    Q_PROPERTY(int launchedTimesRole MEMBER m_launchedTimesRole NOTIFY launchedTimesRoleChanged)
    Q_PROPERTY(int lastLaunchedTimeRole MEMBER m_lastLaunchedTimeRole NOTIFY lastLaunchedTimeRoleChanged)
    QML_NAMED_ELEMENT(FrequentlyUsedProxyModel)
public:
    explicit FrequentlyUsedProxyModel(QObject *parent = nullptr);
    QAbstractItemModel *recentlyInstalledModel() const;
    void setRecentlyInstalledModel(QAbstractItemModel *newRecentlyInstalledModel);

    void classBegin() override;
    void componentComplete() override;

signals:
    void recentlyInstalledModelChanged();
    void desktopIdRoleChanged(int);
    void launchedTimesRoleChanged(int);
    void lastLaunchedTimeRoleChanged(int);

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
    int m_desktopIdRole;
    int m_launchedTimesRole;
    int m_lastLaunchedTimeRole;
};
