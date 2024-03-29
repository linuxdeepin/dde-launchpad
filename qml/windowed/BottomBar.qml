// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import "."

Control {
    id: control

    property alias searchEdit: searchEdit

    contentItem: RowLayout {
        width: parent.width

        ToolButton {
            icon.name: "shutdown"
            ToolTip.visible: hovered
            ToolTip.delay: 1000
            ToolTip.text: qsTr("Power")
            onClicked: {
                DesktopIntegration.openShutdownScreen();
            }
        }

        SearchEdit {
            id: searchEdit

            Layout.leftMargin: parent.width / 4
            Layout.rightMargin: parent.width / 4

            Layout.fillWidth: true
            Layout.preferredHeight: 30
            placeholder: qsTr("Search")
            onTextChanged: {
                console.log(text)
                SearchFilterProxyModel.setFilterRegularExpression(text.trim())
                SearchFilterProxyModel.invalidate()
            }

            property Palette edittingPalette: Palette {
                normal {
                    common: Qt.rgba(0, 0, 0, 0.1)
                }
                normalDark {
                    common: Qt.rgba(1, 1, 1, 0.1)
                }
            }

            property Palette nomalPalette: Palette {
                normal {
                    common: ("transparent")
                }
                normalDark {
                    common: ("transparent")
                }
            }

            backgroundColor: searchEdit.editting ? edittingPalette : nomalPalette
        }

        ToolButton {
            icon.name: "launcher_fullscreen"
            Accessible.name: "Fullscreen"
            onClicked: {
                LauncherController.currentFrame = "FullscreenFrame"
            }
        }
    }

    background: DebugBounding { }
}
