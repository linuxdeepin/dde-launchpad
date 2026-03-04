// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAINTEDDCIICON_H
#define PAINTEDDCIICON_H

#include <QQuickPaintedItem>
#include <QQmlEngine>
#include <DDciIcon>

DGUI_USE_NAMESPACE

class PaintedDciIcon : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground NOTIFY foregroundChanged)
    Q_PROPERTY(QSize sourceSize READ sourceSize WRITE setSourceSize NOTIFY sourceSizeChanged)

public:
    explicit PaintedDciIcon(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    QString name() const;
    void setName(const QString &name);

    QColor foreground() const;
    void setForeground(const QColor &color);

    QSize sourceSize() const;
    void setSourceSize(const QSize &size);

signals:
    void nameChanged();
    void foregroundChanged();
    void sourceSizeChanged();

private:
    QString m_name;
    QColor m_foreground;
    QSize m_sourceSize;
    DDciIcon m_dciIcon;
};

#endif // PAINTEDDCIICON_H
