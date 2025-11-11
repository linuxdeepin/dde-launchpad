// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debughelper.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(logDebugHelper, "org.deepin.dde.launchpad.debug")
}

DebugHelper::DebugHelper(QObject *parent)
    : QObject(parent)
{
    const QString debugSettingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString debugSettingPath(QDir(debugSettingBasePath).absoluteFilePath("debug.ini"));

    m_debugSettings = new QSettings(debugSettingPath, QSettings::NativeFormat, this);
    m_useRegularWindow = m_debugSettings->value("useRegularWindow", false).toBool();
    m_avoidLaunchApp = m_debugSettings->value("avoidLaunchApp", false).toBool();
    m_avoidHideWindow = m_debugSettings->value("avoidHideWindow", false).toBool();
    m_itemBoundingEnabled = m_debugSettings->value("enabledItemBounding", false).toBool();
    qCInfo(logDebugHelper) << "Debug settings loaded - useRegularWindow:" << m_useRegularWindow 
                          << "avoidLaunchApp:" << m_avoidLaunchApp 
                          << "avoidHideWindow:" << m_avoidHideWindow 
                          << "itemBoundingEnabled:" << m_itemBoundingEnabled;

    connect(this, &DebugHelper::onUseRegularWindowChanged, this, [=](bool val){
        m_debugSettings->setValue("useRegularWindow", val);
    });

    connect(this, &DebugHelper::onAvoidLaunchAppChanged, this, [=](bool val){
        m_debugSettings->setValue("avoidLaunchApp", val);
    });

    connect(this, &DebugHelper::onAvoidHideWindowChanged, this, [=](bool val){
        m_debugSettings->setValue("avoidHideWindow", val);
    });

    connect(this, &DebugHelper::onItemBoundingEnabledChanged, this, [=](bool val){
        m_debugSettings->setValue("enabledItemBounding", val);
    });
}

DebugHelper::~DebugHelper()
{
    qCDebug(logDebugHelper) << "Destroying DebugHelper";
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

DebugQuickItem::DebugQuickItem(QObject *parent)
    : QObject(parent)
{
    static int gOffset = 0;
    gOffset += 25;
    QColor gray(Qt::gray);
    gray.setGreen((gray.green() + gOffset) % 255);
    gray.setBlue((gray.blue() + gOffset) % 255);
    setColor(gray);
    qCDebug(logDebugHelper) << "DebugQuickItem color set to:" << gray;
}

DebugQuickItem *DebugQuickItem::qmlAttachedProperties(QObject *object)
{
    return new DebugQuickItem(object);
}

QColor DebugQuickItem::color() const
{
    return m_color;
}

void DebugQuickItem::setColor(const QColor &newColor)
{
    if (m_color == newColor)
        return;
    m_color = newColor;
    emit colorChanged();
}
