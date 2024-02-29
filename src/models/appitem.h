// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QStandardItem>

class AppItem : public QStandardItem
{
    Q_GADGET
    QML_NAMED_ELEMENT(AppItem)
    QML_UNCREATABLE("AppItem should only be created from C++ side")
public:
    enum Roles {
        DesktopIdRole = Qt::UserRole,
        Categories,
        DDECategoryRole,
        IconNameRole,
        NameRole,
        InstalledTimeRole,
        LastLaunchedTimeRole,
        LaunchedTimesRole,
        ModelExtendedRole = 0x1000
    };
    Q_ENUM(Roles)

    // This is different from the menu-spec Main Categories list.
    enum DDECategories {
        Internet,               // 网络模式
        Chat,                   // 社交模式
        Music,                  // 音乐模式
        Video,                  // 视频模式
        Graphics,               // 图形图像
        Game,                   //
        Office,                 // 办公模式
        Reading,                // 阅读模式
        Development,            // 编程开发模式
        System,                 // 系统管理模式
        Others,
    };
    Q_ENUM(DDECategories)

public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // FIXME: why do we need this one?
    explicit AppItem() { Q_ASSERT_X(false, "AppItem()", "qmlRegisterUncreatableType require this for unknown reason in Qt6, another ctor should be used"); }
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    explicit AppItem(const QString &freedesktopId);
    ~AppItem();

    const QString freedesktopId() const;
    const QString name() const;
    void setName(const QString & name);
    const QString displayName() const;
    void setDisplayName(const QString & name);
    const QString iconName() const;
    void setIconName(const QString & iconName);
    const QStringList categories() const;
    void setCategories(const QStringList & categories);
    DDECategories ddeCategory() const;
    void setDDECategory(DDECategories category);
    qint64 installedTime() const;
    void setInstalledTime(qint64 time);
    qint64 lastLaunchedTime() const;
    void setLastLaunchedTime(qint64 time);
    qint64 launchedTimes() const;
    void setLaunchedTimes(qint64 times);
    void updateData(const AppItem * appItem);
};

Q_DECLARE_METATYPE(AppItem::DDECategories);
