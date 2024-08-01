// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "personalizationmanager.h"
#include "personalizationwindow.h"

#include "qwayland-treeland-personalization-manager-v1.h"

#include <QGuiApplication>
#include <QWindow>

#include <private/qwaylandwindow_p.h>

class PersonalizationManagerPrivate : public QWaylandClientExtensionTemplate<PersonalizationManagerPrivate>,
                                      public QtWayland::treeland_personalization_manager_v1
{
    Q_OBJECT
public:
    explicit PersonalizationManagerPrivate();
    bool personalizeWindow(QWindow * window, PersonalizationManager::BgState state);

private:
    bool doPersonalizeWindow(QWindow * window, PersonalizationManager::BgState state);
    QList<std::pair<QWindow *, PersonalizationManager::BgState> > m_queue;
};


PersonalizationManagerPrivate::PersonalizationManagerPrivate()
    : QWaylandClientExtensionTemplate<PersonalizationManagerPrivate>(1)
{
    connect(this, &PersonalizationManagerPrivate::activeChanged, this, [this](){
        if (!isActive()) return;
        for (std::pair<QWindow *, PersonalizationManager::BgState> task_pair : std::as_const(m_queue)) {
            doPersonalizeWindow(task_pair.first, task_pair.second);
        }
        m_queue.clear();
    });
}

// return if the window is "personalized" right away.
bool PersonalizationManagerPrivate::personalizeWindow(QWindow * window, PersonalizationManager::BgState state)
{
    if (!isActive()) {
        m_queue.append(std::make_pair(window, state));
        return false;
    } else {
        doPersonalizeWindow(window, state);
        return true;
    }
}

// return if success
bool PersonalizationManagerPrivate::doPersonalizeWindow(QWindow * window, PersonalizationManager::BgState state)
{
    Q_ASSERT(isActive());
    if (window && window->handle()) {
        QtWaylandClient::QWaylandWindow *waylandWindow =
            static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
        struct wl_surface *surface = waylandWindow->wlSurface();
        if (surface) {
            PersonalizationWindow *context =
                new PersonalizationWindow(get_window_context(surface));
            context->set_background_type(state);
            qDebug() << "Applied background type" << state << "to window" << window;
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------

PersonalizationManager::PersonalizationManager(QObject * parent)
    : QObject(parent)
{
    if (QGuiApplication::platformName() == "wayland") {
        m_dptr = new PersonalizationManagerPrivate;
    }
}

PersonalizationManager::~PersonalizationManager()
{
    if (m_dptr) delete m_dptr;
}

// return if the window is "personalized" right away.
bool PersonalizationManager::personalizeWindow(QWindow * window, PersonalizationManager::BgState state)
{
    return m_dptr ? m_dptr->personalizeWindow(window, state) : true;
}

#include "personalizationmanager.moc"
