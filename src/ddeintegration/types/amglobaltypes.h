// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: CC0-1.0

#pragma once

#include <QMap>
#include <QDBusObjectPath>
#include <QDBusArgument>

using ObjectInterfaceMap = QMap<QString, QVariantMap>;
using ObjectMap = QMap<QDBusObjectPath, ObjectInterfaceMap>;
using QStringMap = QMap<QString, QString>;
using PropMap = QMap<QString, QStringMap>;

Q_DECLARE_METATYPE(ObjectInterfaceMap)
Q_DECLARE_METATYPE(ObjectMap)
Q_DECLARE_METATYPE(QStringMap)
Q_DECLARE_METATYPE(PropMap)

inline void registerComplexDbusType()  // FIXME: test shouldn't associate with DBus
{
    qRegisterMetaType<ObjectInterfaceMap>();
    qDBusRegisterMetaType<ObjectInterfaceMap>();
    qRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<QStringMap>();
    qRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<QDBusObjectPath>();
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, QStringMap &map)
{
    argument.beginMap();
    while (!argument.atEnd()) {
        QString key;
        QString value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        map.insert(key, value);
    }
    argument.endMap();
    return argument;
}
