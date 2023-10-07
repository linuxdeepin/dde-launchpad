// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

Control {
    id: root

    width: 96
    height: 96

    property var icons: undefined
    property int preferredIconSize: 48

    property string iconSource

    Accessible.name: iconItemLabel.text

    signal folderClicked()
    signal itemClicked()
    signal menuTriggered()

    contentItem: Rectangle {
//        anchors.fill: parent
        color: (stylus.hovered/* || parent.focus */) ? Qt.rgba(0, 0, 0, 0.3) : "transparent"
        radius: 18

        Behavior on color { PropertyAnimation {} }

        Column {
            anchors.fill: parent

            Rectangle {
                // actually just a top padding
                color: "transparent"
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

            Rectangle {
                width: root.width / 2
                height: root.height / 2
                anchors.horizontalCenter: parent.horizontalCenter
                radius: 8
                color: root.icons ? Qt.rgba(0, 0, 0, 0.5) : "transparent"

                Loader {
                    anchors.fill: parent
                    sourceComponent: root.icons !== undefined ? folderComponent : imageComponent
                }

                Component {
                    id: folderComponent

                    Grid {
                        id: folderGrid
                        anchors.fill: parent
                        rows: 2
                        columns: 2
                        spacing: 5
                        padding: 5

                        Repeater {
                            model: icons
                            delegate: Rectangle {
                                visible: true
                                color: "transparent"
                                width: (folderGrid.width - (folderGrid.columns - 1) * folderGrid.spacing - folderGrid.padding * 2) / folderGrid.columns
                                height: (folderGrid.height - (folderGrid.rows - 1) * folderGrid.spacing - folderGrid.padding * 2) / folderGrid.rows

                                Image {
                                    anchors.fill: parent
                                    source: "image://app-icon/" + modelData
                                    sourceSize: Qt.size(parent.width, parent.height)
                                }
                            }
                        }
                    }
                }

                Component {
                    id: imageComponent

                    Image {
                        id: iconImage
                        anchors.fill: parent
                        source: iconSource
                        sourceSize: Qt.size(parent.width, parent.height)
                    }
                }
            }


            Label {
                id: iconItemLabel
                text: display
                textFormat: Text.PlainText
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                maximumLineCount: 2
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: {
            root.menuTriggered()
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (root.icons) {
                root.folderClicked()
            } else {
                root.itemClicked()
            }
        }
        onPressAndHold: {

        }
    }

    HoverHandler {
        id: stylus
    }

    Keys.onSpacePressed: {
        root.itemClicked()
    }

    Keys.onReturnPressed: {
        root.itemClicked()
    }
}
