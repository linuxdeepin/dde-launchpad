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

    width: 96
    height: 96

    property var icons: undefined
    property int preferredIconSize: 48
    property string text: display.startsWith("internal/category/") ? getCategoryName(display.substring(18)) : display

    property string iconSource
    property bool dndEnabled: false

    Accessible.name: iconItemLabel.text

    signal folderClicked()
    signal itemClicked()

    contentItem: ToolButton {
        focusPolicy: Qt.NoFocus
        contentItem: Column {
            anchors.fill: parent

            Item {
                // actually just a top padding
                width: root.width
                height: root.height / 9
            }

            Rectangle {
                visible: false
                anchors.right: parent.right

                color: "blue"

                width: 6
                height: 6
                radius: width / 2
            }

            Item {
                width: 48
                height: width
                anchors.horizontalCenter: parent.horizontalCenter

                Loader {
                    id: iconLoader
                    anchors.fill: parent
                    sourceComponent: root.icons !== undefined ? folderComponent : imageComponent
                }

                Component {
                    id: folderComponent

                    Image {
                        id: iconImage
                        anchors.fill: parent
                        source: "image://folder-icon/" + icons.join(':')
                        sourceSize: Qt.size(parent.width, parent.height)
                    }
                }

                Component {
                    id: imageComponent

                    DciIcon {
                        objectName: "appIcon"
                        anchors.fill: parent
                        name: iconSource
                        sourceSize: Qt.size(parent.width, parent.height)
                    }
                }
            }

            // as topMargin
            Item {
                width: 1
                height: 8
            }

            Label {
                id: iconItemLabel
                text: root.text
                textFormat: Text.PlainText
                width: parent.width
                leftPadding: 2
                rightPadding: 2
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                elide: Text.ElideMiddle
                maximumLineCount: 2
                font: DTK.fontManager.t8
            }    
        }
        background: ButtonPanel {
            button: parent
            outsideBorderColor: null
            radius: 8
        }

        onClicked: {
            if (root.icons) {
                root.folderClicked()
            } else {
                root.itemClicked()
            }
        }
    }
}
