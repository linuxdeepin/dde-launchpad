// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "x11keymaskfixer.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>
#include <xcb/xcb.h>

void X11KeyMaskFixer::install()
{
    static X11KeyMaskFixer *self = nullptr;
    if (!self) {
        self = new X11KeyMaskFixer;
        QCoreApplication::instance()->installNativeEventFilter(self);
    }
}

void X11KeyMaskFixer::fixWindow(QWindow *window)
{
    const QString platform = QGuiApplication::platformName();
    if (platform != QLatin1String("dxcb") && platform != QLatin1String("xcb"))
        return;

    auto *nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface)
        return;
    auto *connection = static_cast<xcb_connection_t *>(
        nativeInterface->nativeResourceForIntegration("connection"));
    if (!connection || !window)
        return;

    const xcb_window_t winId = window->winId();
    xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes_unchecked(connection, winId);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(connection, cookie, nullptr);
    if (!reply)
        return;
    const unsigned mask = reply->your_event_mask;
    free(reply);

    const unsigned keyboardMask = XCB_EVENT_MASK_KEY_PRESS
                                  | XCB_EVENT_MASK_KEY_RELEASE
                                  | XCB_EVENT_MASK_FOCUS_CHANGE;
    if ((mask & keyboardMask) == keyboardMask)
        return;

    // OR in the missing masks, keeping whatever this client already selected.
    const uint32_t newMask = mask | keyboardMask
                             | XCB_EVENT_MASK_BUTTON_PRESS
                             | XCB_EVENT_MASK_BUTTON_RELEASE
                             | XCB_EVENT_MASK_BUTTON_MOTION
                             | XCB_EVENT_MASK_ENTER_WINDOW
                             | XCB_EVENT_MASK_LEAVE_WINDOW
                             | XCB_EVENT_MASK_POINTER_MOTION
                             | XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(connection, winId, XCB_CW_EVENT_MASK, &newMask);
    xcb_flush(connection);
}

bool X11KeyMaskFixer::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    if (eventType != QByteArrayLiteral("xcb_generic_event_t") || !message)
        return false;

    auto *event = static_cast<xcb_generic_event_t *>(message);
    xcb_window_t windowId = 0;
    switch (event->response_type & 0x7f) {
    case XCB_MAP_NOTIFY:
        windowId = reinterpret_cast<xcb_map_notify_event_t *>(event)->window;
        break;
    case XCB_CONFIGURE_NOTIFY:
        windowId = reinterpret_cast<xcb_configure_notify_event_t *>(event)->window;
        break;
    case XCB_FOCUS_IN:
    case XCB_FOCUS_OUT:
        windowId = reinterpret_cast<xcb_focus_in_event_t *>(event)->event;
        break;
    default:
        return false;
    }

    const auto windows = QGuiApplication::topLevelWindows();
    for (QWindow *window : windows) {
        if (window->isVisible() && window->winId() == windowId) {
            fixWindow(window);
            break;
        }
    }
    return false;
}

static void installX11KeyMaskFixer()
{
    X11KeyMaskFixer::install();
}

Q_COREAPP_STARTUP_FUNCTION(installX11KeyMaskFixer)
