// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launchercontroller.h"

#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <DGuiApplicationHelper>
#include <QCommandLineParser>
#include <launcher1adaptor.h>

DGUI_USE_NAMESPACE

LauncherController::LauncherController(QObject *parent)
    : QObject(parent)
    , optShow(QStringList{"s", "show"}, tr("Show launcher (hidden by default)"))
    , optToggle(QStringList{"t", "toggle"}, tr("Toggle launcher visibility"))
    , m_timer(new QTimer(this))
    , m_launcher1Adaptor(new Launcher1Adaptor(this))
    , m_visible(false)
{
    // TODO: settings should be managed in somewhere else.
    const QString settingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString settingPath(QDir(settingBasePath).absoluteFilePath("settings.ini"));
    QSettings settings(settingPath, QSettings::NativeFormat);

    m_currentFrame = settings.value("current_frame", "WindowedFrame").toString();
    if (qgetenv("DDE_CURRENT_COMPOSITOR") == "TreeLand") {
        m_currentFrame = QStringLiteral("FullscreenFrame");
    }

    // Interval set to 500=>1000ms for issue https://github.com/linuxdeepin/developer-center/issues/8137
    m_timer->setInterval(1000);
    m_timer->setSingleShot(true);

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

    // for dbus adapter signals.
    connect(this, &LauncherController::visibleChanged, this, [this](bool isVisible){
        if (isVisible) {
            emit Shown();
        } else {
            emit Closed();
        }
        emit VisibleChanged(isVisible);
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
    if (m_timer->isActive()) {
        qDebug() << "hit";
        m_timer->stop();
        return;
    }
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

bool LauncherController::isFullScreenFrame() const
{
    return m_currentFrame == QStringLiteral("FullscreenFrame");
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
    qDebug() << "set current frame:" << m_currentFrame;
    emit currentFrameChanged();
}

// We need to hide the launcher when it lost focus, but clicking the launcher icon on the taskbar/dock will also trigger
// `Toggle()`, which will show the launcher even if it just get hid caused by losting focus. Thus, we added a timer to
// mark it as we just hide it, and check if the timer is running while calling `Toggle()`. This function will do nothing
// if it's already hidden (`Toggle()` get triggered before `hideWithTimer()` get called).
void LauncherController::hideWithTimer()
{
    if (visible()) {
        m_timer->start();
        qDebug() << "hide with timer";
        setVisible(false);
    }
}

QFont LauncherController::adjustFontWeight(const QFont &f, QFont::Weight weight)
{
    QFont font(f);
    font.setWeight(weight);
    return font;
}
