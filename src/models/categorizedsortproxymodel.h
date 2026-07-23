// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>

class CategorizedSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(CategoryType categoryType READ categoryType WRITE setCategoryType NOTIFY categoryTypeChanged)
    Q_PROPERTY(QString sortRoleName READ sortRoleName NOTIFY categoryTypeChanged)
    Q_PROPERTY(QString sectionRoleName READ sectionRoleName NOTIFY sectionRoleNameChanged)

    QML_NAMED_ELEMENT(CategorizedSortProxyModel)
    QML_SINGLETON
public:
    static CategorizedSortProxyModel &instance()
    {
        static CategorizedSortProxyModel _instance;
        return _instance;
    }

    static CategorizedSortProxyModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)

        return &instance();
    }

    enum CategoryType {
        Alphabetary,
        DDECategory,
        FreeCategory,
    };
    Q_ENUM(CategoryType)

    void setCategoryType(enum CategoryType categoryType);
    enum CategoryType categoryType() const;

    QString sortRoleName() const;
    QString sectionRoleName() const { return m_sectionRoleName; }

    Q_INVOKABLE QList<QString> alphabetarySections() const;
    Q_INVOKABLE QList<int> DDECategorySections() const;

signals:
    void categoryTypeChanged();
    void sectionRoleNameChanged();

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    bool isFreeSort;
    QString m_sectionRoleName;
    explicit CategorizedSortProxyModel(QObject *parent = nullptr);
};
