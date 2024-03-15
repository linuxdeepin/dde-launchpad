// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//import QtCore
import QtQuick 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

Loader {
    id: control
    anchors.fill: parent
    asynchronous: true
    active: DebugHelper.itemBoundingEnabled
    sourceComponent: Rectangle {
        color: "transparent"
        border.color: control.DebugItem.color
    }
}
