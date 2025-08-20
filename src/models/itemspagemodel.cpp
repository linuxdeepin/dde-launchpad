// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemspagemodel.h"
#include "itemarrangementproxymodel.h"
#include "itemspage.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logModels)

ItemsPageModel::ItemsPageModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_topLevel(nullptr)
    , m_sourceModel(nullptr)
{
    qCDebug(logModels) << "Initializing ItemsPageModel";
}

int ItemsPageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    
    qCDebug(logModels) << "Getting row count";
    if (!m_topLevel) {
        qCDebug(logModels) << "No top level page, returning 0";
        return 0;
    }

    const auto& pageCount = m_topLevel->pageCount();
    qCDebug(logModels) << "Row count:" << pageCount;
    return pageCount;
}

QVariant ItemsPageModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    qCDebug(logModels) << "Getting data for index" << index.row() << "role" << role;
    return QVariant();
}

void ItemsPageModel::setSourceModel(QAbstractItemModel *model)
{
    qCDebug(logModels) << "Setting source model:" << model;
    if (!model || model == m_sourceModel) {
        qCDebug(logModels) << "Invalid model or same model, skipping";
        return;
    }

    m_sourceModel = model;
    qCInfo(logModels) << "Source model changed, emitting signal";
    emit sourceModelChanged(model);

    ItemsPage *topLevel = qobject_cast<ItemArrangementProxyModel*>(m_sourceModel)->itemsPage();
    qCDebug(logModels) << "Got top level items page:" << topLevel;
    
    if (!topLevel || m_topLevel.data() == topLevel) {
        qCDebug(logModels) << "Same or invalid top level page, skipping";
        return;
    } else if (m_topLevel && m_topLevel.data() != topLevel) {
        qCDebug(logModels) << "Disconnecting signals from old top level page";
        m_topLevel->disconnect(SIGNAL(sigPageAdded(int, int)), this);
        m_topLevel->disconnect(SIGNAL(sigPageRemoved(int, int)), this);
    }

    m_topLevel = topLevel;
    qCDebug(logModels) << "Setting new top level page and connecting signals";
    
    connect(m_topLevel, &ItemsPage::sigPageAdded, this, [ this ] (int first, int last) {
        qCDebug(logModels) << "Page added signal received:" << first << "to" << last;
        beginInsertRows(QModelIndex(), first, last);
        endInsertRows();
    });

    connect(m_topLevel, &ItemsPage::sigPageRemoved, this, [ this ] (int first, int last) {
        qCDebug(logModels) << "Page removed signal received:" << first << "to" << last;
        beginRemoveRows(QModelIndex(), first, last);
        endRemoveRows();
    });
    
    qCInfo(logModels) << "ItemsPageModel setup completed with new top level page";
}
