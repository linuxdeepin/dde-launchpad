// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef INPUTEVENTITEM_H
#define INPUTEVENTITEM_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

class InputEventItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QQuickItem* inputMethodSource READ inputMethodSource WRITE setInputMethodSource NOTIFY inputMethodSourceChanged)
public:
    InputEventItem();

    QQuickItem* inputMethodSource() const;
    void setInputMethodSource(QQuickItem* source);

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void inputReceived(const QString &input);
    void inputMethodSourceChanged();

private:
    QQuickItem* m_inputMethodSource = nullptr;
};

#endif // INPUTEVENTITEM_H
