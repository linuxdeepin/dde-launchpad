// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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

QQuickItem* InputEventItem::inputMethodSource() const
{
    return m_inputMethodSource;
}

void InputEventItem::setInputMethodSource(QQuickItem* source)
{
    if (m_inputMethodSource != source) {
        m_inputMethodSource = source;
        Q_EMIT inputMethodSourceChanged();
    }
}

QVariant InputEventItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (m_inputMethodSource) {
        QVariant result = m_inputMethodSource->inputMethodQuery(query);
        if (query == Qt::ImCursorRectangle) {
            QRectF rect = result.toRectF();
            QPointF mapped = m_inputMethodSource->mapToItem(this, rect.topLeft());
            rect.moveTopLeft(mapped);
            return rect;
        }
        if (query == Qt::ImEnabled) {
            return true;
        }
        return result;
    }
    return QQuickItem::inputMethodQuery(query);
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
