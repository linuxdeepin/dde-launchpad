// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launchercontroller.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <DGuiApplicationHelper>
#include <QCommandLineParser>
#include <DRegionMonitor>

DGUI_USE_NAMESPACE

LauncherController::LauncherController(QObject *parent)
    : QObject(parent)
    , optShow(QStringList{"s", "show"}, tr("Show launcher (hidden by default)"))
    , optToggle(QStringList{"t", "toggle"}, tr("Toggle launcher visibility"))
    , m_regionMonitor(new DRegionMonitor(this))
    , m_visible(false)
{
    // TODO: settings should be managed in somewhere else.
    const QString settingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString settingPath(QDir(settingBasePath).absoluteFilePath("settings.ini"));
    QSettings settings(settingPath, QSettings::NativeFormat);

    m_currentFrame = settings.value("current_frame", "WindowedFrame").toString();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance,
            this, [this](qint64 pid, const QStringList & args) {
        Q_UNUSED(pid)

        QCommandLineParser parser;

        parser.addOption(optShow);
        parser.addOption(optToggle);
        parser.parse(args);

        if (parser.isSet(optShow)) {
            setVisible(true);
        } else if (parser.isSet(optToggle)) {
            setVisible(!visible());
        }
    });

    // for dbus adapter signals, AND for m_regionMonitor.
    connect(this, &LauncherController::visibleChanged, this, [this](bool isVisible){
        if (isVisible) {
            m_regionMonitor->registerRegion();
            emit Shown();
        } else {
            m_regionMonitor->unregisterRegion();
            emit Closed();
        }
        emit VisibleChanged(isVisible);
    });

    // since the launcher window is tend to be x11bypassed
    m_regionMonitor->setCoordinateType(Dtk::Gui::DRegionMonitor::Original);

    connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, [](const QPoint &p, const int flag){
        qDebug() << p << flag << "do we really need x11bypass just for hidding launcher window?";
    });
}

void LauncherController::Exit()
{
    qApp->quit();
}

void LauncherController::Hide()
{
    setVisible(false);
}

void LauncherController::Show()
{
    setVisible(true);
}

void LauncherController::ShowByMode(qlonglong in0)
{
    Q_UNUSED(in0)
    // the original launcher implementation did nothing while calling this dbus API
    // I guess we can deprecate this API.
}

void LauncherController::Toggle()
{
    setVisible(!visible());
}

LauncherController::~LauncherController()
{

}

bool LauncherController::visible() const
{
    return m_visible;
}

void LauncherController::setVisible(bool visible)
{
    if (visible == m_visible) return;

    m_visible = visible;

    emit visibleChanged(m_visible);
}

QString LauncherController::currentFrame() const
{
    return m_currentFrame;
}

void LauncherController::setCurrentFrame(const QString &frame)
{
    if (m_currentFrame == frame) return;

    const QString settingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString settingPath(QDir(settingBasePath).absoluteFilePath("settings.ini"));
    QSettings settings(settingPath, QSettings::NativeFormat);

    settings.setValue("current_frame", frame);

    m_currentFrame = frame;
    emit currentFrameChanged();
}
