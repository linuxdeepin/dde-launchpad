// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QtWaylandClient/QWaylandClientExtension>

#include "qwayland-xdg-activation-v1.h"

class QWindow;

namespace DDEIntegration {

// Token provider: wraps the xdg_activation_token_v1 object and emits done()
// when the compositor delivers the token.
class XdgActivationTokenV1 : public QObject, public QtWayland::xdg_activation_token_v1
{
    Q_OBJECT
public:
    ~XdgActivationTokenV1() override;

Q_SIGNALS:
    void done(const QString &token);

protected:
    void xdg_activation_token_v1_done(const QString &token) override;
};

// Client extension: binds to the xdg_activation_v1 global and allows
// requesting activation tokens.
class XdgActivationV1 : public QWaylandClientExtensionTemplate<XdgActivationV1>,
                        public QtWayland::xdg_activation_v1
{
    Q_OBJECT
public:
    // Returns the process-wide singleton instance (created on first call).
    static XdgActivationV1 *instance();

    ~XdgActivationV1() override;

    // Synchronously request a token (blocks with a nested event loop until the
    // compositor delivers it or the 2-second timeout elapses).
    // Returns an empty string when not running on Wayland or when the
    // compositor does not expose the extension.
    QString requestToken(QWindow *window, const QString &appId);

private:
    explicit XdgActivationV1();
};

} // namespace DDEIntegration
