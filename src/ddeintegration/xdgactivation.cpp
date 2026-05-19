// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xdgactivation.h"

#include <QEventLoop>
#include <QTimer>
#include <QWindow>
#include <QLoggingCategory>

#include <private/qwaylandwindow_p.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandinputdevice_p.h>

Q_DECLARE_LOGGING_CATEGORY(logDdeIntegration)

namespace DDEIntegration {

// ---------------------------------------------------------------------------
// XdgActivationTokenV1
// ---------------------------------------------------------------------------

XdgActivationTokenV1::~XdgActivationTokenV1()
{
    destroy();
}

void XdgActivationTokenV1::xdg_activation_token_v1_done(const QString &token)
{
    Q_EMIT done(token);
}

// ---------------------------------------------------------------------------
// XdgActivationV1
// ---------------------------------------------------------------------------

XdgActivationV1 *XdgActivationV1::instance()
{
    static XdgActivationV1 s_instance;
    return &s_instance;
}

XdgActivationV1::XdgActivationV1()
    : QWaylandClientExtensionTemplate<XdgActivationV1>(1)
{
}

XdgActivationV1::~XdgActivationV1()
{
    if (isInitialized())
        destroy();
}

QString XdgActivationV1::requestToken(QWindow *window, const QString &appId)
{
    if (!isActive()) {
        qCWarning(logDdeIntegration) << "xdg_activation_v1 is not active, cannot request token";
        return {};
    }

    auto *provider = new XdgActivationTokenV1;
    provider->init(get_activation_token());

    // Attach the surface and input serial of the requesting window so the
    // compositor can verify focus and apply focus-stealing-prevention rules.
    if (window) {
        if (auto *waylandWindow =
                dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle())) {
            if (auto *surface = waylandWindow->wlSurface()) {
                provider->set_surface(surface);
            }
            // set_serial tells the compositor which input event triggered this
            // launch request; without it the compositor may deny focus for the
            // new window (focus-stealing prevention).
            if (auto *inputDevice = waylandWindow->display()->lastInputDevice()) {
                provider->set_serial(inputDevice->serial(), inputDevice->wl_seat());
            }
        }
    }

    if (!appId.isEmpty())
        provider->set_app_id(appId);

    provider->commit();

    // Block until the compositor delivers the token or the timeout fires.
    QString token;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(2000);

    connect(provider, &XdgActivationTokenV1::done, &loop,
            [&token, &loop](const QString &t) {
                token = t;
                loop.quit();
            });
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    timeout.start();
    loop.exec();

    if (token.isEmpty())
        qCWarning(logDdeIntegration) << "XDG activation token request timed out";
    else
        qCDebug(logDdeIntegration) << "Received XDG activation token for app:" << appId;

    provider->deleteLater();
    return token;
}

} // namespace DDEIntegration
