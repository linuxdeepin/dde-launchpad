// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include "dsglobal.h"

namespace dock {

class LauncherItem : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(QString iconName MEMBER m_iconName NOTIFY iconNameChanged FINAL)
public:
    explicit LauncherItem(QObject *parent = nullptr);
    virtual bool init() override;

Q_SIGNALS:
    void iconNameChanged();

private:
    QString m_iconName;
};

}
