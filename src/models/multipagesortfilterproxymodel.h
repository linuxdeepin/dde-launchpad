// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class MultipageSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)
    Q_PROPERTY(int folderId MEMBER m_folderId NOTIFY onFolderIdChanged)
    Q_PROPERTY(int pageId MEMBER m_pageId NOTIFY onPageIdChanged)
    Q_PROPERTY(bool filterOnlyMode MEMBER m_filterOnlyMode NOTIFY onFilterOnlyModeChanged)
    QML_NAMED_ELEMENT(MultipageSortFilterProxyModel)

public:
    explicit MultipageSortFilterProxyModel(QObject *parent = nullptr);
    ~MultipageSortFilterProxyModel();

    void setModel(QAbstractItemModel *model);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

signals:
    void onFolderIdChanged(int);
    void onPageIdChanged(int);
    void sourceModelChanged(QObject *);
    void onFilterOnlyModeChanged(bool);

private:
    int m_folderId;
    int m_pageId;
    bool m_filterOnlyMode;
};
