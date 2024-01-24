// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
    readonly property bool isWindowedMode: LauncherController.currentFrame === "WindowedFrame"

    Accessible.name: iconItemLabel.text

    signal folderClicked()
    signal itemClicked()
    signal menuTriggered()

    Drag.dragType: Drag.Automatic
    // Drag.active: dragHandler.active

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
                width: isWindowedMode ? 48 : parent.width / 2
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
                height: isWindowedMode ? 8 : 20
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
            radius: isWindowedMode ? 8 : 18
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
                // We switch to use the `dndItem` to handle Drag event since that one will always exists.
                // If we use the current item, then if the item that provides the drag attached property
                // get destoryed (e.g. switch page or folder close caused destory), dropping at that moment
                // will cause a crash.

                // Item will be hidden by checking the dndItem.currentlyDraggedId property. We assign the value
                // to that property here
                dndItem.currentlyDraggedId = parent.Drag.mimeData["text/x-dde-launcher-dnd-desktopId"]
                // TODO: This way we couldn't give it an image size hint,
                dndItem.Drag.imageSource = icons ? ("image://folder-icon/" + icons.join(':')) : ("image://app-icon/" + DTK.platformTheme.iconThemeName + "/" + parent.iconSource)
                Qt.callLater(function() {
                    dndItem.Drag.mimeData = parent.Drag.mimeData
                    dndItem.Drag.active = true
                    dndItem.Drag.startDrag()
                })
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
