// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class RecentlyInstalledProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(RecentlyInstalledProxyModel)
    Q_PROPERTY(int lastLaunchedTimeRole MEMBER m_lastLaunchedTimeRole NOTIFY lastLaunchedTimeRoleChanged)
    Q_PROPERTY(int installedTimeRole MEMBER m_installedTimeRole NOTIFY installedTimeRoleChanged)
public:
    explicit RecentlyInstalledProxyModel(QObject *parent = nullptr);

    void classBegin() override;
    void componentComplete() override;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

signals:
    void lastLaunchedTimeRoleChanged();
    void installedTimeRoleChanged();

private:
    int m_lastLaunchedTimeRole;
    int m_installedTimeRole;
};
