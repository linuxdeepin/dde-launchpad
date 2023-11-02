// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

ColumnLayout {
    id: root

    signal backClicked()
    signal categoryClicked(string character)

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: false
        Layout.preferredHeight: 50
        spacing: 16

        ToolButton {
            icon.name: "back"
            Accessible.name: "Back"
            onClicked: {
                root.backClicked();
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        DelegateModel {
            id: delegateAlphabetCategoryModel
            model: [
                'A', 'B', 'C', 'D',
                'E', 'F', 'G', 'H',
                'I', 'J', 'K', 'L',
                'M', 'N', 'O', 'P',
                'Q', 'R', 'S', 'T',
                'U', 'V', 'W', 'X',
                'Y', 'Z', '&', '#',
            ]
            delegate: Rectangle {
                width: alphabetCategoryContainer.cellSize
                height: alphabetCategoryContainer.cellSize
                color: (stylus.hovered/* || parent.focus */) ? Qt.rgba(0, 0, 0, 0.3) : "transparent"
                radius: 10
                enabled: modelData !== 'F'

                Label {
                    anchors.fill: parent
                    text: modelData
                    textFormat: Text.PlainText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                HoverHandler {
                    id: stylus
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        categoryClicked(modelData)
                    }
                }
            }
        }

        GridViewContainer {
            id: alphabetCategoryContainer

            width: parent.width
            height: parent.height * 5 / 6

            anchors.centerIn: parent
            activeGridViewFocusOnTab: true
            model: delegateAlphabetCategoryModel
            rows: 7
            columns: 4
            padding: 10
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: false
        Layout.preferredHeight: 50
        spacing: 16

        ToolButton {
            icon.name: "shutdown"
            ToolTip.visible: hovered
            ToolTip.delay: 1000
            ToolTip.text: qsTr("Power")
            onClicked: {
                DesktopIntegration.openShutdownScreen();
            }
        }

        ToolButton {
            icon.name: "setting"
            Accessible.name: qsTr("Settings")
            onClicked: {
                DesktopIntegration.openSystemSettings();
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
