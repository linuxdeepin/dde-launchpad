// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMSPAGEMODEL_H
#define ITEMSPAGEMODEL_H

#include <QtQml/qqml.h>
#include <QAbstractListModel>

#include "itemspage.h"

class ItemsPageModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
    QML_NAMED_ELEMENT(ItemsPageModel)

public:
    explicit ItemsPageModel(QObject *parent = nullptr);

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QAbstractItemModel *sourceModel() { return m_sourceModel; }
    void setSourceModel(QAbstractItemModel *model);

signals:
    void sourceModelChanged(QObject *);

private:
    QPointer<ItemsPage> m_topLevel;
    QAbstractItemModel *m_sourceModel;
};

#endif // ITEMSPAGEMODEL_H
