// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FREE_SORT_PROXY_MODEL_H
#define FREE_SORT_PROXY_MODEL_H

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class FreeSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)
    QML_NAMED_ELEMENT(FreeSortProxyModel)

public:
    explicit FreeSortProxyModel(QObject *parent = nullptr);

    void setModel(QAbstractItemModel *model);

signals:
    void sourceModelChanged(QObject *);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

#endif //FREE_SORT_PROXY_MODEL_H
