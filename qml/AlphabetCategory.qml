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
            delegate: ToolButton {
                width: alphabetCategoryContainer.cellSize
                height: alphabetCategoryContainer.cellSize
                text: modelData
                focusPolicy: Qt.NoFocus
                onClicked: {
                    categoryClicked(modelData)
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
