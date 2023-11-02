// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

MouseArea {
    id: root

    property alias contentItem: control.contentItem

    onClicked: {
        console.log("drawer clicked")
        root.visible = false
    }

    Control {
        id: control

        anchors.centerIn: parent

        width: contentItem.width
        height: contentItem.height

        background: InWindowBlur {
            id: blur
            anchors.fill: parent
            radius: 20
            offscreen: true

            ItemViewport {
                anchors.fill: parent
                fixed: true
                sourceItem: parent
                radius: control.radius
                hideSource: false
            }

            BoxShadow {
                anchors.fill: backgroundRect
                shadowOffsetX: 0
                shadowOffsetY: 6
                shadowColor: control.ColorSelector.dropShadowColor
                shadowBlur: 20
                cornerRadius: backgroundRect.radius
                spread: 0
                hollow: true
            }

            Rectangle {
                id: backgroundRect
                anchors.fill: parent
                radius: control.radius
                color: "transparent"//control.ColorSelector.backgroundColor
            }
        }
    }
}
