// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QStandardItem>

class AppItem : public QStandardItem
{
    Q_GADGET
public:
    enum Roles {
        DesktopIdRole = Qt::UserRole,
        Categories,
        DDECategoryRole,
        IconNameRole,
        ModelExtendedRole = 0x1000
    };

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
    explicit AppItem(const QString &freedesktopId);
    ~AppItem();

    const QString freedesktopId() const;
    const QString displayName() const;
    void setDisplayName(const QString & name);
    const QString iconName() const;
    void setIconName(const QString & iconName);
    const QStringList categories() const;
    void setCategories(const QStringList & categories);
    DDECategories ddeCategory() const;
    void setDDECategory(DDECategories category);
    void updateData(const AppItem * appItem);
};

Q_DECLARE_METATYPE(AppItem::DDECategories);
