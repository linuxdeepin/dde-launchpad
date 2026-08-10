// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "dsglobal.h"

#include <QAbstractItemModel>
#include <QVariant>

namespace dock {

class LauncherItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(QString iconName MEMBER m_iconName NOTIFY iconNameChanged FINAL)
    Q_PROPERTY(QAbstractItemModel *itemArrangementModel READ itemArrangementModel CONSTANT FINAL)
public:
    explicit LauncherItem(QObject *parent = nullptr);
    virtual bool init() override;

    QAbstractItemModel *itemArrangementModel() const;
    Q_INVOKABLE QString groupDisplayName(const QString &groupId) const;
    Q_INVOKABLE QVariantList groupItemDetails(const QString &groupId) const;

Q_SIGNALS:
    void iconNameChanged();

private:
    QString m_iconName;
};

}
