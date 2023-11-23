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
    property bool dndEnabled: false

    Accessible.name: iconItemLabel.text

    signal folderClicked()
    signal itemClicked()
    signal menuTriggered()

    Drag.dragType: Drag.Automatic
    Drag.active: dragHandler.active

    states: State {
        name: "dragged";
        when: dragHandler.active
        // FIXME: When dragging finished, the position of the item is changed for unknown reason,
        //        so we use the state to reset the x and y here.
        PropertyChanges {
            target: root
            x: x
            y: y
        }
    }

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
                width: root.width / 2
                height: root.height / 2
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
                text: display.startsWith("internal/category/") ? getCategoryName(display.substring(18)) : display
                textFormat: Text.PlainText
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                maximumLineCount: 2
            }    
        }

        onClicked: {
            if (root.icons) {
                root.folderClicked()
            } else {
                root.itemClicked()
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: {
            root.menuTriggered()
        }
    }

    DragHandler {
        id: dragHandler
        enabled: root.dndEnabled

        onActiveChanged: {
            if (active) {
                // TODO: 1. this way we couldn't give it an image size hint,
                //       2. also not able to set offset to drag image, so the cursor is always
                //          at the top-left of the image
                //       3. we should also handle folder icon
                parent.Drag.imageSource = icons ? ("image://folder-icon/" + icons.join(':')) : parent.iconSource
            }
        }
    }

    Keys.onSpacePressed: {
        if (root.icons !== undefined) {
            root.folderClicked()
        } else {
            root.itemClicked()
        }
    }

    Keys.onReturnPressed: {
        if (root.icons !== undefined) {
            root.folderClicked()
        } else {
            root.itemClicked()
        }
    }
}
