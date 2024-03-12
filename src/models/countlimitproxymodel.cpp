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

bool CountLimitProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_maxRowCount < 0)
        return true;

    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    if (!modelIndex.isValid())
        return false;

    return modelIndex.row() < m_maxRowCount;
}
