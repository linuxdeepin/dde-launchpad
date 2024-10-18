// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0
import org.deepin.dtk.private 1.0

import org.deepin.launchpad 1.0

Control {
    id: root

    property string text: display.startsWith("internal/category/") ? getCategoryName(display.substring(18)) : display

    property string iconSource
    property bool dndEnabled: false

    Accessible.name: iconItemLabel.text

    signal itemClicked()
    signal menuTriggered()

    contentItem: Button {
        id: iconButton
        focusPolicy: Qt.NoFocus
        ColorSelector.pressed: false
        ColorSelector.family: Palette.CrystalColor
        flat: true
        contentItem: Column {
            anchors.fill: parent

            Item {
                // actually just a top padding
                width: 1
                height: 7
            }

            DciIcon {
                objectName: "appIcon"
                anchors.horizontalCenter: parent.horizontalCenter
                name: iconSource
                sourceSize: Qt.size(36, 36)
                palette: DTK.makeIconPalette(root.palette)
                theme: DTK.toColorType(root.palette.window)

                DciIcon {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    name: "emblem_autostart"
                    visible: autoStart
                    sourceSize: Qt.size(12, 12)
                    palette: DTK.makeIconPalette(root.palette)
                    theme: ApplicationHelper.DarkType
                }
            }

            // as topMargin
            Item {
                width: 1
                height: 4
            }

            Label {
                property bool singleRow: font.pixelSize > Helper.windowed.doubleRowMaxFontSize
                id: iconItemLabel
                text: root.text
                textFormat: Text.PlainText
                width: parent.width
                leftPadding: 2
                rightPadding: 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                wrapMode: singleRow ? Text.NoWrap : Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: singleRow ? 1 : 2
                font: DTK.fontManager.t9
            }
        }
        ToolTip.text: root.text
        ToolTip.delay: 500
        ToolTip.visible: hovered && iconItemLabel.truncated
        background: ItemBackground {
            radius: 8
            button: iconButton
        }

        onClicked: {
            root.itemClicked()
        }
    }
    background: DebugBounding { }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: {
            root.menuTriggered()
        }
    }

    Keys.onSpacePressed: {
        root.itemClicked()
    }

    Keys.onReturnPressed: {
        root.itemClicked()
    }
}
