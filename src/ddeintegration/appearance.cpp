// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearance.h"

#include <QGuiApplication>
#include <QScreen>
#include <QImage>

#include "Appearance1.h"
#include "blurhash.hpp"

using Appearance1 = __Appearance1;

Appearance::Appearance(QObject *parent)
    : QObject(parent)
    , m_dbusAppearanceIface(new Appearance1(Appearance1::staticInterfaceName(), "/org/deepin/dde/Appearance1",
                                            QDBusConnection::sessionBus(), this))
    , m_wallpaperBlurhash("L35?hb%#0ADeorNFVuy501Me?*%o")
{
    QTimer::singleShot(0, this, &Appearance::updateCurrentWallpaperBlurhash);

    connect(m_dbusAppearanceIface, &Appearance1::Changed, this, [this](const QString & key, const QString &) {
        if (key == "allwallpaperuris") updateCurrentWallpaperBlurhash();
    });
    if (m_dbusAppearanceIface->isValid()) {
        connect(m_dbusAppearanceIface, &Appearance1::OpacityChanged, this, [this](double value) {
            setOpacity(value);
        });
        setOpacity(m_dbusAppearanceIface->opacity());
    }

    connect(&m_blurhashWatcher, &QFutureWatcher<QString>::finished, this, [this](){
        QString result(m_blurhashWatcher.result());
        if (!result.isEmpty()) {
            m_wallpaperBlurhash = result;
            emit wallpaperBlurhashChanged();
        }
    });
}

Appearance::~Appearance()
{

}

QString Appearance::wallpaperBlurhash() const
{
    return m_wallpaperBlurhash;
}

void Appearance::updateCurrentWallpaperBlurhash()
{
    const QString screenName = qApp->primaryScreen()->name();

    QDBusPendingReply<QString> async = m_dbusAppearanceIface->GetCurrentWorkspaceBackgroundForMonitor(screenName);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* call){
        QDBusPendingReply<QString> reply = *call;
        if (reply.isError()) {
            qDebug() << "Cannot get wallpaper from dbus:" << reply.error();
        } else {
            QUrl wallpaperUrl(reply.value());
            qDebug() << "Got wallpaper URL from dbus:" << wallpaperUrl;

            QFuture<QString> blurhashFuture = QtConcurrent::run([wallpaperUrl](){
                QImage image;
                if (image.load(wallpaperUrl.toLocalFile())) {
                    image.convertTo(QImage::Format_RGB888);
                    std::string blurhash = blurhash::encode(image.constBits(), image.width(), image.height(), 4, 3);
                    QString newBlurhash(QString::fromStdString(blurhash));
                    return newBlurhash;
                } else {
                    return QString();
                }
            });
            m_blurhashWatcher.setFuture(blurhashFuture);
        }
    });
}

qreal Appearance::opacity() const
{
    return m_opacity;
}

void Appearance::setOpacity(qreal newOpacity)
{
    if (qFuzzyCompare(m_opacity, newOpacity))
        return;
    m_opacity = newOpacity;
    emit opacityChanged();
}
