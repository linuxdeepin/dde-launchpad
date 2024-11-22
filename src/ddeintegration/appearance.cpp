// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearance.h"

#include <QGuiApplication>
#include <QScreen>
#include <QImage>

#include "Appearance1.h"
#include "blurhash.hpp"
#include "../launchercontroller.h"

using Appearance1 = __Appearance1;

Appearance::Appearance(QObject *parent)
    : QObject(parent)
    , m_dbusAppearanceIface(new Appearance1(Appearance1::staticInterfaceName(), "/org/deepin/dde/Appearance1",
                                            QDBusConnection::sessionBus(), this))
    , m_wallpaperBlurhash("L35?hb%#0ADeorNFVuy501Me?*%o")
{
    QTimer::singleShot(0, this, &Appearance::updateAllWallpaper);

    connect(m_dbusAppearanceIface, &Appearance1::Changed, this, [this](const QString & key, const QString &) {
        if (key == "allwallpaperuris") updateAllWallpaper();
    });

    connect(&(LauncherController::instance()), &LauncherController::currentFrameChanged,
            this, &Appearance::updateCurrentWallpaperBlurhash);
    connect(&(LauncherController::instance()), &LauncherController::visibleChanged,
            this, &Appearance::updateCurrentWallpaperBlurhash);

    if (m_dbusAppearanceIface->isValid()) {
        connect(m_dbusAppearanceIface, &Appearance1::OpacityChanged, this, [this](double value) {
            setOpacity(value);
        });
        setOpacity(m_dbusAppearanceIface->opacity());
    }
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
    if (!LauncherController::instance().visible() || !LauncherController::instance().isFullScreenFrame())
        return;

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
            if (m_wallpaperBlurMap.contains(wallpaperUrl)) {
                m_wallpaperBlurhash = m_wallpaperBlurMap.value(wallpaperUrl);
                emit wallpaperBlurhashChanged();
            } else {
                // try update new workspace background image
                updateAllWallpaper();
            }
        }
    });
}

void Appearance::updateAllWallpaper()
{
    QJsonParseError err;
    QString urls = m_dbusAppearanceIface->wallpaperURls();
    QJsonDocument doc = QJsonDocument::fromJson(urls.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "get wallpapers failed" << err.errorString();
        return;
    }

    if (!doc.isObject()) {
        return;
    }

    auto obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QString k = it.key();
        QJsonValue v = it.value();
#ifdef QT_DEBUG
        qDebug() << k << ":" << v;
#endif

        if (!v.isString())
            break;

        QUrl wallpaperUrl(v.toString());
        if (m_wallpaperBlurMap.contains(wallpaperUrl))
            continue;

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

        QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>;
        watcher->setFuture(blurhashFuture);
        connect(watcher, &QFutureWatcher<QString>::finished, this, [this, wallpaperUrl](){
            auto watcher = static_cast<QFutureWatcher<QString>*>(sender());
            if (!watcher)
                return;

            QString result(watcher->result());
            if (!result.isEmpty() && !m_wallpaperBlurMap.contains(wallpaperUrl)) {
                m_wallpaperBlurhash = result;
                m_wallpaperBlurMap[wallpaperUrl] = result;
                updateCurrentWallpaperBlurhash();
            }

            watcher->deleteLater();
            m_blurhashWatchers.removeOne(watcher);
        });

        m_blurhashWatchers << watcher;
    }
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
