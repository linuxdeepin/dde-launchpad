// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

namespace Dtk::Core {
class DConfig;
}

class SearchFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SearchFilterProxyModel)
    QML_SINGLETON
public:
    static SearchFilterProxyModel &instance()
    {
        static SearchFilterProxyModel _instance;
        return _instance;
    }

    static SearchFilterProxyModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &instance();
    }

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    explicit SearchFilterProxyModel(QObject *parent = nullptr);

    int calculateWeight(const QModelIndex &modelIndex) const;

    Dtk::Core::DConfig *m_dconfig;
    bool m_searchPackageEnabled;
};
