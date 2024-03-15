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
            placeholder: qsTr("Search")
            onTextChanged: {
                console.log(text)
                SearchFilterProxyModel.setFilterRegularExpression(text)
                SearchFilterProxyModel.invalidate()
            }

            Palette {
                id: actionPalette
                normal {
                    common: ("#f7f7f7")
                    crystal: Qt.rgba(0, 0, 0, 0.1)
                }
                normalDark {
                    common: Qt.rgba(1, 1, 1, 0.1)
                    crystal: Qt.rgba(1, 1, 1, 0.1)
                }
                hovered {
                    common: ("#e1e1e1")
                    crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
                }
                pressed {
                    common: ("#bcc4d0")
                    crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
                }
            }

            Palette {
                id: nomalPalette
                normal {
                    common: ("transparent")
                    crystal: Qt.rgba(0, 0, 0, 0.1)
                }
                normalDark {
                    common: ("transparent")
                    crystal: Qt.rgba(1, 1, 1, 0.1)
                }
                hovered {
                    common: ("#e1e1e1")
                    crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
                }
                pressed {
                    common: ("#bcc4d0")
                    crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
                }
            }

            backgroundColor: searchEdit.editting ? actionPalette : nomalPalette
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
