// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QDBusMessage>
#include <QDBusConnection>
#include <QLoggingCategory>

#include <DConfig>

#ifdef HAVE_DDE_API_EVENTLOGGER
#include <dde-api/eventlogger.hpp>
#endif

#include <private/qguiapplication_p.h>

DGUI_USE_NAMESPACE

namespace {
Q_LOGGING_CATEGORY(logController, "org.deepin.dde.launchpad.controller")

constexpr qint64 EVENT_LOGGER_LAUNCHPAD_MODE = 1000610012;
constexpr auto EnableLaunchpadKey = "enableLaunchpad";

void logLaunchpadMode(const QString &mode, const char *description)
{
#ifdef HAVE_DDE_API_EVENTLOGGER
    DDE_EventLogger::EventLogger::instance().writeEventLog(
        DDE_EventLogger::EventLoggerData(EVENT_LOGGER_LAUNCHPAD_MODE, QStringLiteral("launchpad_config"), {
            {QStringLiteral("launchpad_mode"), mode}
        }));
#endif
    qCInfo(logController) << "EventLogger: launchpad mode" << description << ":" << mode;
}
}

LauncherController::LauncherController(QObject *parent)
    : QObject(parent)
    , optShow(QStringList{"s", "show"}, tr("Show launcher (hidden by default)"))
    , optToggle(QStringList{"t", "toggle"}, tr("Toggle launcher visibility"))
    , m_timer(new QTimer(this))
    , m_launcher1Adaptor(new Launcher1Adaptor(this))
    , m_launchpadConfig(Dtk::Core::DConfig::create(QStringLiteral("org.deepin.dde.shell"),
                                                   QStringLiteral("org.deepin.ds.launchpad"),
                                                   QString(),
                                                   this))
    , m_visible(false)
    , m_enabled(true)
{
    if (m_launchpadConfig) {
        connect(m_launchpadConfig, &Dtk::Core::DConfig::valueChanged, this, [this](const QString &key) {
            if (key == QLatin1String(EnableLaunchpadKey)) {
                updateEnabled();
            }
        });
        updateEnabled();
    } else {
        qCWarning(logController) << "Failed to create launchpad DConfig; launchpad remains enabled";
    }

    // TODO: settings should be managed in somewhere else.
    const QString settingBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    const QString settingPath(QDir(settingBasePath).absoluteFilePath("settings.ini"));
    QSettings settings(settingPath, QSettings::NativeFormat);

    m_currentFrame = settings.value("current_frame", "WindowedFrame").toString();
    qCInfo(logController) << "Current frame mode:" << m_currentFrame;

    logLaunchpadMode(m_currentFrame, "on startup");

    // Interval set to 500=>1000ms for issue https://github.com/linuxdeepin/developer-center/issues/8137
    m_timer->setInterval(1000);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this] {
        if (m_pendingHide) {
            m_pendingHide = false;
            setVisible(false);
        }
    });

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
        m_pendingHide = false;
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
    if (visible && !m_enabled) {
        qCDebug(logController) << "Ignoring launchpad show request because it is disabled by DConfig";
        return;
    }

    if (visible == m_visible) return;

    m_visible = visible;

    emit visibleChanged(m_visible);
}

bool LauncherController::enabled() const
{
    return m_enabled;
}

void LauncherController::updateEnabled()
{
    if (!m_launchpadConfig->isValid()) {
        qCWarning(logController) << "Launchpad DConfig is invalid; launchpad remains enabled";
        return;
    }

    const bool enabled = m_launchpadConfig->value(QLatin1String(EnableLaunchpadKey), true).toBool();
    if (enabled == m_enabled) {
        return;
    }

    m_enabled = enabled;
    if (!m_enabled) {
        setVisible(false);
    }

    qCInfo(logController) << "Launchpad enabled state changed:" << m_enabled;
    emit enabledChanged(m_enabled);
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

    logLaunchpadMode(m_currentFrame, "changed to");

    m_pendingHide = false;
    m_timer->start();
    emit currentFrameChanged();
}

QString LauncherController::currentScreen() const
{
    return m_currentScreen;
}

void LauncherController::setCurrentScreen(const QString &screen)
{
    if (m_currentScreen == screen) return;

    m_currentScreen = screen;
    qCInfo(logController) << "Current screen changed to:" << m_currentScreen;
    emit currentScreenChanged();
}

// We need to hide the launcher when it lost focus, but clicking the launcher icon on the taskbar/dock will also trigger
// `Toggle()`, which will show the launcher even if it just get hid caused by losting focus. Thus, we added a timer to
// mark it as we just hide it, and check if the timer is running while calling `Toggle()`. This function will do nothing
// if it's already hidden (`Toggle()` get triggered before `hideWithTimer()` get called).
void LauncherController::hideWithTimer()
{
    if (visible()) {
        if (m_timer->isActive()) {
            m_pendingHide = true;
            return;
        }
        if (m_avoidHide) {
            qDebug() << "hide with timer";
            setVisible(false);
        }
    }
}

void LauncherController::cancelHide()
{
    m_pendingHide = false;
}

QFont LauncherController::adjustFontWeight(const QFont &f, QFont::Weight weight)
{
    QFont font(f);
    font.setWeight(weight);
    return font;
}

void LauncherController::closeAllPopups()
{
    QGuiApplicationPrivate *qAppPrivate = QGuiApplicationPrivate::instance();
    Q_ASSERT(qAppPrivate);
    qAppPrivate->closeAllPopups();
}

void LauncherController::setAvoidHide(bool avoidHide)
{
    m_avoidHide = avoidHide;
}

void LauncherController::showHelp()
{
    // 由于当前只有调用 “启动器”，才能跳转到帮助文档的启动器目录。使用launcher 以及launchpad等字段，无法跳转到启动器目录。
    QString helpTitle = "启动器";
    
    const QString &dmanInterface = "com.deepin.Manual.Open";
    QDBusMessage message = QDBusMessage::createMethodCall(dmanInterface, "/com/deepin/Manual/Open", dmanInterface, "OpenTitle");
    message << "dde" << helpTitle;
    QDBusConnection::sessionBus().asyncCall(message);
}

//首次从全屏切换到窗口时候，会出现焦点丢失抖动问题，从而导致启动器窗口不显示，所以采用此方法处理。
void LauncherController::setCurrentFrameToWindowedFrame()
{
    setVisible(false);
    QTimer::singleShot(100, this, [this]() {
        setCurrentFrame("WindowedFrame");
        setVisible(true);
    });
}
