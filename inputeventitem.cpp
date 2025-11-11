// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "inputeventitem.h"
#include <QLoggingCategory>
namespace {
Q_LOGGING_CATEGORY(logInputEvent, "org.deepin.dde.launchpad.input")
}

InputEventItem::InputEventItem()
{
    qApp->installEventFilter(this);
}

bool InputEventItem::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::InputMethod && (this->children().contains(obj) || obj == this)) {
        QInputMethodEvent *inputMethodEvent = static_cast<QInputMethodEvent *>(event);
        qCDebug(logInputEvent) << "Input method event received:" << inputMethodEvent->commitString();
        if (!inputMethodEvent->commitString().isEmpty()) {
            qCInfo(logInputEvent) << "Emitting input received signal:" << inputMethodEvent->commitString();
            Q_EMIT inputReceived(inputMethodEvent->commitString());
        }
    }
    return QObject::eventFilter(obj, event);
}
