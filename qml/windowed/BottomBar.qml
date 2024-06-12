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

    property Item keyTabTarget: shutdownBtn
    property Item nextKeyTabTarget
    property alias searchEdit: searchEdit

    padding: 10

    contentItem: RowLayout {
        ToolButton {
            id: shutdownBtn
            icon.name: "shutdown"
            background: ItemBackground {
               button: shutdownBtn
            }
            ToolTip.visible: hovered
            ToolTip.delay: 1000
            ToolTip.text: qsTr("Power")
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            onClicked: {
                DesktopIntegration.openShutdownScreen();
            }
        }

        // TODO dtk's bug, ColorSelector's control is wrong for SearchEdit.
        Control {
            Layout.preferredWidth: 360
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.preferredHeight: 30

            contentItem: SearchEdit {
                id: searchEdit
                padding: 1
                property Palette iconPalette: Palette {
                    normal {
                        crystal: Qt.rgba(0, 0, 0, 1)
                    }
                    normalDark {
                        crystal: Qt.rgba(1, 1, 1, 1)
                    }
                }
                placeholder: qsTr("Search")
                placeholderTextColor: palette.brightText
                palette.windowText: ColorSelector.iconPalette

                onTextChanged: {
                    console.log(text)
                    SearchFilterProxyModel.setFilterRegularExpression(text.trim())
                    SearchFilterProxyModel.invalidate()
                }

                property Palette edittingPalette: Palette {
                    normal {
                        crystal: Qt.rgba(0, 0, 0, 0.1)
                    }
                    normalDark {
                        crystal: Qt.rgba(1, 1, 1, 0.1)
                    }
                }

                property Palette nomalPalette: Palette {
                    normal {
                        crystal: ("transparent")
                    }
                    normalDark {
                        crystal: ("transparent")
                    }
                    hovered {
                        crystal:  Qt.rgba(0, 0, 0, 0.05)
                    }
                    hoveredDark {
                        crystal:  Qt.rgba(1, 1, 1, 0.05)
                    }
                }

                backgroundColor: searchEdit.editting ? edittingPalette : nomalPalette
            }
        }

        ToolButton {
            id: fullscreenBtn
            icon.name: "launcher_fullscreen"
            background: ItemBackground {
                button: fullscreenBtn
            }
            Accessible.name: "Fullscreen"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: {
                searchEdit.text = ""
                LauncherController.currentFrame = "FullscreenFrame"
            }
            KeyNavigation.tab: nextKeyTabTarget
        }
    }

    background: DebugBounding { }
}
