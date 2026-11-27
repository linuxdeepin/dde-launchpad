// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>

class QWindow;

// On X11 the dxcb platform plugin creates override-redirect popup windows
// with an event mask that lacks KeyPressMask/KeyReleaseMask/FocusChangeMask.
// X11 delivers keyboard events per-client based on the event mask each client
// selected, so without those masks no key event ever reaches Qt, while mouse
// events (delivered through XInput2) keep working. This silently breaks text
// input in any popup window of the process, e.g. the launcher search box.
//
// This helper installs a native event filter on QCoreApplication startup and
// restores the keyboard/focus event masks through Qt's own xcb connection
// whenever one of our top-level windows gets mapped or focused. The masks are
// per-client, so only Qt's own event delivery is affected.
class X11KeyMaskFixer : public QAbstractNativeEventFilter
{
public:
    static void install();

    // Read the window's current event mask and OR in the keyboard, focus and
    // pointer masks if they are missing. Idempotent, safe to call repeatedly.
    static void fixWindow(QWindow *window);

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message,
                           qintptr *result) override;
};
