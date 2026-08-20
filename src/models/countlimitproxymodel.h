// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class CountLimitProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int maxRowCount READ maxRowCount WRITE setMaxRowCount NOTIFY maxRowCountChanged FINAL)
    QML_NAMED_ELEMENT(CountLimitProxyModel)
public:

    int maxRowCount() const;
    void setMaxRowCount(int newMaxRowCount);

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
signals:
    void maxRowCountChanged();
private:
    int m_maxRowCount = -1;
};
