// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "itemspagemodel.h"
#include "itemarrangementproxymodel.h"

ItemsPageModel::ItemsPageModel(QObject *parent) : QAbstractListModel(parent), m_sourceModel(nullptr) {}
int ItemsPageModel::rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : m_count; }
QVariant ItemsPageModel::data(const QModelIndex &, int) const { return {}; }
void ItemsPageModel::setSourceModel(QAbstractItemModel *model)
{
    if (model == m_sourceModel && (model || !m_count)) return;
    if (m_sourceModel) disconnect(m_sourceModel, nullptr, this, nullptr);
    m_sourceModel = model;
    auto arrangement = qobject_cast<ItemArrangementProxyModel *>(model);
    const auto update = [this, arrangement] {
        const int count = arrangement ? arrangement->pageCount() : 0;
        if (m_count == count) return;
        if (count > m_count) {
            beginInsertRows({}, m_count, count - 1);
            m_count = count;
            endInsertRows();
        } else {
            beginRemoveRows({}, count, m_count - 1);
            m_count = count;
            endRemoveRows();
        }
    };
    if (arrangement) {
        connect(arrangement, &ItemArrangementProxyModel::topLevelPageCountChanged, this, update);
        connect(arrangement, &QObject::destroyed, this, [this] { setSourceModel(nullptr); });
    }
    update();
    emit sourceModelChanged(model);
}
