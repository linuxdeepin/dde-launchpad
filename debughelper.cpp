// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debughelper.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>

DebugHelper::DebugHelper(QObject *parent)
    : QObject(parent)
{
    const QString debugSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString debugSettingPath(QDir(debugSettingBasePath).absoluteFilePath("debug.ini"));

    m_debugSettings = new QSettings(debugSettingPath, QSettings::NativeFormat, this);
    m_useRegularWindow = m_debugSettings->value("useRegularWindow", false).toBool();
    m_avoidLaunchApp = m_debugSettings->value("avoidLaunchApp", false).toBool();
    m_avoidHideWindow = m_debugSettings->value("avoidHideWindow", false).toBool();

    connect(this, &DebugHelper::onUseRegularWindowChanged, this, [=](bool val){
        m_debugSettings->setValue("useRegularWindow", val);
    });

    connect(this, &DebugHelper::onAvoidLaunchAppChanged, this, [=](bool val){
        m_debugSettings->setValue("avoidLaunchApp", val);
    });

    connect(this, &DebugHelper::onAvoidHideWindowChanged, this, [=](bool val){
        m_debugSettings->setValue("avoidHideWindow", val);
    });
}

DebugHelper::~DebugHelper()
{

}

// check if QT_DEBUG is defined
bool DebugHelper::qtDebugEnabled() const
{
#ifdef QT_DEBUG
    return true;
#else
    return false;
#endif
}
