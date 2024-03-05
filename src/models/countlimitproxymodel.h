// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QSortFilterProxyModel>

class CountLimitProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int maxRowCount READ maxRowCount WRITE setMaxRowCount NOTIFY maxRowCountChanged FINAL)
public:

    int maxRowCount() const;
    void setMaxRowCount(int newMaxRowCount);
    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const;
signals:
    void maxRowCountChanged();
private:
    int m_maxRowCount = -1;
};
