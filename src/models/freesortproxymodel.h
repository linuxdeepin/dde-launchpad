// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FREE_SORT_PROXY_MODEL_H
#define FREE_SORT_PROXY_MODEL_H

#include <QQmlEngine>

#include "sortproxymodel.h"

class FreeSortProxyModel : public SortProxyModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(FreeSortProxyModel)
public:
    explicit FreeSortProxyModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

#endif //FREE_SORT_PROXY_MODEL_H
