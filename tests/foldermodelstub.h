// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QStandardItemModel>
#include <QStringList>
#include <QVariantMap>

// Minimal stand-in for the dde-apps shared folder model. ItemArrangementProxyModel
// only talks to it through QMetaObject::invokeMethod, so a QStandardItemModel with
// matching Q_INVOKABLEs is enough for the unit tests.
class FolderModelStub : public QStandardItemModel
{
    Q_OBJECT
public:
    int pages = 2;
    int emptyPages = 0;
    QStringList lastMove;
    QStringList broughtToFront;
    QHash<QString, QVariantMap> positions;

    Q_INVOKABLE QVariantMap folderInfo(const QString &id)
    {
        return {{"id", id}, {"name", id}, {"contents", QVariantList{}}};
    }
    Q_INVOKABLE QVariantMap itemPosition(const QString &id)
    {
        return positions.value(id, {{"group", 0}, {"page", 0}, {"pos", 0}});
    }
    Q_INVOKABLE int pageCount(int) { return pages; }
    Q_INVOKABLE void updateFolderName(int, const QString &) { emit arrangementChanged(); }
    Q_INVOKABLE void bringToFront(const QString &id)
    {
        broughtToFront.append(id);
        positions[id] = {{"group", 0}, {"page", 0}, {"pos", 0}};
        emit arrangementChanged();
    }
    Q_INVOKABLE void rearrange(const QString &drag, const QString &drop, int, int)
    {
        lastMove = {drag, drop};
        emit arrangementChanged();
    }
    Q_INVOKABLE int createEmptyPage(int)
    {
        const int page = pages++;
        ++emptyPages;
        emit arrangementChanged();
        return page;
    }
    Q_INVOKABLE void removeEmptyPages()
    {
        pages -= emptyPages;
        emptyPages = 0;
        emit arrangementChanged();
    }

signals:
    void arrangementChanged();
};
