// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "painteddciicon.h"

#include <QPainter>
#include <QQuickWindow>
#include <QGuiApplication>
#include <DDciIconPalette>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

PaintedDciIcon::PaintedDciIcon(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setRenderTarget(QQuickPaintedItem::Image);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() { update(); });
}

void PaintedDciIcon::paint(QPainter *painter)
{
    if (m_dciIcon.isNull())
        return;

    qreal dpr = window() ? window()->devicePixelRatio() : qApp->devicePixelRatio();
    auto appTheme = DGuiApplicationHelper::instance()->themeType();
    DDciIcon::Theme theme = (appTheme == DGuiApplicationHelper::DarkType)
                                ? DDciIcon::Dark : DDciIcon::Light;

    QPalette pa = qApp->palette();
    DDciIconPalette palette(
        m_foreground.isValid() ? m_foreground : pa.windowText().color(),
        pa.window().color(),
        pa.highlight().color(),
        pa.highlightedText().color()
    );

    int iconW = m_sourceSize.isValid() ? m_sourceSize.width() : qRound(width());
    int iconH = m_sourceSize.isValid() ? m_sourceSize.height() : qRound(height());
    QRect iconRect(0, 0, iconW, iconH);
    QRect itemRect(0, 0, qRound(width()), qRound(height()));
    iconRect.moveCenter(itemRect.center());
    m_dciIcon.paint(painter, iconRect, dpr, theme, DDciIcon::Normal,
                    Qt::AlignCenter, palette);
}

QString PaintedDciIcon::name() const
{
    return m_name;
}

void PaintedDciIcon::setName(const QString &name)
{
    if (m_name == name)
        return;
    m_name = name;
    m_dciIcon = DDciIcon::fromTheme(m_name);
    emit nameChanged();
    update();
}

QColor PaintedDciIcon::foreground() const
{
    return m_foreground;
}

void PaintedDciIcon::setForeground(const QColor &color)
{
    if (m_foreground == color)
        return;
    m_foreground = color;
    emit foregroundChanged();
    update();
}

QSize PaintedDciIcon::sourceSize() const
{
    return m_sourceSize;
}

void PaintedDciIcon::setSourceSize(const QSize &size)
{
    if (m_sourceSize == size)
        return;
    m_sourceSize = size;
    emit sourceSizeChanged();
    update();
}
