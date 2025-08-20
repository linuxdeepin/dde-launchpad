// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "countlimitproxymodel.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logModels)

int CountLimitProxyModel::maxRowCount() const
{
    return m_maxRowCount;
}

void CountLimitProxyModel::setMaxRowCount(int newMaxRowCount)
{
    if (m_maxRowCount == newMaxRowCount) {
        qCDebug(logModels) << "Max row count unchanged, skipping update";
        return;
    }
    const auto& oldValue = m_maxRowCount;
    m_maxRowCount = newMaxRowCount;
    qCInfo(logModels) << "Max row count changed from" << oldValue << "to" << newMaxRowCount;
    emit maxRowCountChanged();
    invalidate();
}

void CountLimitProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (sourceModel == this->sourceModel())
        return;

    static const struct {
        const char *signalName;
        const char *slotName;
    } connectionTable[] = {
                           { SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(invalidate()) },
                           { SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(invalidate()) },
                           };

    if (this->sourceModel()) {
        for (const auto &c : connectionTable)
            disconnect(this->sourceModel(), c.signalName, this, c.slotName);
    }

    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (sourceModel) {
        for (const auto &c : connectionTable) {
            connect(sourceModel, c.signalName, this, c.slotName);
        }
    }
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
