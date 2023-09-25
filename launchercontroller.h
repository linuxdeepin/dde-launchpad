// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCommandLineOption>
#include <QObject>

namespace Dtk {
namespace Gui {
class DRegionMonitor;
}
}

class QTimer;
class Launcher1Adaptor;
class LauncherController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QString currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)

    // I really don't want to expose those dbus API as public function...
    friend class Launcher1Adaptor;

public:

    static LauncherController &instance()
    {
        static LauncherController _instance;
        return _instance;
    }

    ~LauncherController();

    bool visible() const;
    void setVisible(bool visible);
    QString currentFrame() const;
    void setCurrentFrame(const QString & frame);

    Q_INVOKABLE void hideWithTimer();

signals:
    void currentFrameChanged();
    void visibleChanged(bool visible);

public:
    QCommandLineOption optShow;
    QCommandLineOption optToggle;

    // called by dbus adapter
private:
    Q_PROPERTY(bool Visible READ visible NOTIFY VisibleChanged)
    void Exit();
    void Hide();
    void Show();
    void ShowByMode(qlonglong in0);
    void Toggle();
signals:
    void Closed();
    void Shown();
    void VisibleChanged(bool visible);

private:
    explicit LauncherController(QObject *parent=nullptr);

    QTimer *m_timer;
    Dtk::Gui::DRegionMonitor *m_regionMonitor;
    Launcher1Adaptor * m_launcher1Adaptor;
    bool m_visible;
    QString m_currentFrame;
};
