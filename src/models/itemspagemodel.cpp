// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemspagemodel.h"
#include "itemarrangementproxymodel.h"
#include "itemspage.h"

ItemsPageModel::ItemsPageModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_topLevel(nullptr)
    , m_sourceModel(nullptr)
{

}

int ItemsPageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (!m_topLevel) {
        return 0;
    }

    return m_topLevel->pageCount();
}

QVariant ItemsPageModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return QVariant();
}

void ItemsPageModel::setSourceModel(QAbstractItemModel *model)
{
    if (!model || model == m_sourceModel) {
        return;
    }

    m_sourceModel = model;
    emit sourceModelChanged(model);

    ItemsPage *topLevel = qobject_cast<ItemArrangementProxyModel*>(m_sourceModel)->itemsPage();
    if (!topLevel || m_topLevel.data() == topLevel) {
        return;
    } else if (m_topLevel && m_topLevel.data() != topLevel) {
        m_topLevel->disconnect(SIGNAL(sigPageAdded(int, int)), this);
        m_topLevel->disconnect(SIGNAL(sigPageRemoved(int, int)), this);
    }

    m_topLevel = topLevel;
    connect(m_topLevel, &ItemsPage::sigPageAdded, this, [ this ] (int first, int last) {
        beginInsertRows(QModelIndex(), first, last);
        endInsertRows();
    });

    connect(m_topLevel, &ItemsPage::sigPageRemoved, this, [ this ] (int first, int last) {
        beginRemoveRows(QModelIndex(), first, last);
        endRemoveRows();
    });
}
