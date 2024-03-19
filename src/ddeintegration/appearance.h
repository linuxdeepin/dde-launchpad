// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QtConcurrent>

class __Appearance1;
class Appearance : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString wallpaperBlurhash READ wallpaperBlurhash NOTIFY wallpaperBlurhashChanged)
    Q_PROPERTY(qreal opacity READ opacity NOTIFY opacityChanged FINAL)

public:
    explicit Appearance(QObject *parent = nullptr);
    ~Appearance();

    QString wallpaperBlurhash() const;

    qreal opacity() const;
    void setOpacity(qreal newOpacity);

signals:
    void wallpaperBlurhashChanged();

    void opacityChanged();

private:
    void updateCurrentWallpaperBlurhash();

    __Appearance1 * m_dbusAppearanceIface;

    QString m_wallpaperBlurhash;
    QFutureWatcher<QString> m_blurhashWatcher;
    qreal m_opacity = -1;
};
