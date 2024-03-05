// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "countlimitproxymodel.h"

int CountLimitProxyModel::maxRowCount() const
{
    return m_maxRowCount;
}

void CountLimitProxyModel::setMaxRowCount(int newMaxRowCount)
{
    if (m_maxRowCount == newMaxRowCount)
        return;
    m_maxRowCount = newMaxRowCount;
    emit maxRowCountChanged();
    invalidate();
}

int CountLimitProxyModel::rowCount(const QModelIndex &parent) const
{
    if (m_maxRowCount <= 0)
        return QSortFilterProxyModel::rowCount(parent);

    return qMin(QSortFilterProxyModel::rowCount(parent), m_maxRowCount);
}
