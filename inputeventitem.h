// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
public:
    InputEventItem();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void inputReceived(const QString &input);
};

#endif // INPUTEVENTITEM_H
