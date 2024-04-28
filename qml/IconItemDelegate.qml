// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0
import org.deepin.dtk.private 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Control {
    id: root

    property var icons: undefined
    property int preferredIconSize: 48
    property string text: display.startsWith("internal/category/") ? getCategoryName(display.substring(18)) : display

    property string iconSource
    property bool dndEnabled: false
    readonly property bool isWindowedMode: LauncherController.currentFrame === "WindowedFrame"
    property alias displayFont: iconItemLabel.font

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
                height: isWindowedMode ? 3 : root.height / 9
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
                width: parent.width / 2
                height: width
                anchors.horizontalCenter: parent.horizontalCenter

                Loader {
                    id: iconLoader
                    Behavior on scale {
                        PropertyAnimation {
                            duration: 100
                            easing.type: Easing.OutQuad
                        }
                    }
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
                        palette: DTK.makeIconPalette(root.palette)
                        theme: ApplicationHelper.DarkType
                    }
                }
            }

            // as topMargin
            Item {
                width: 1
                height: isWindowedMode ? 5 : root.height / 10
            }

            Label {
                property bool singleRow: isWindowedMode && (font.pixelSize > Helper.windowed.doubleRowMaxFontSize)
                id: iconItemLabel
                text: root.text
                textFormat: Text.PlainText
                width: parent.width
                leftPadding: 2
                rightPadding: 2
                horizontalAlignment: Text.AlignHCenter
                wrapMode: singleRow ? Text.NoWrap : Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: singleRow ? 1 : 2
            }
        }
        ToolTip.text: root.text
        ToolTip.delay: 1000
        ToolTip.visible: hovered
        background: ButtonPanel {
            button: parent
            outsideBorderColor: null
            radius: isWindowedMode ? 8 : 18
        }

        // Must place this handler in contentItem, cause contentItem will grab Qt.LeftButton
        TapHandler {
            acceptedButtons: Qt.LeftButton
            longPressThreshold: 0.3 // 0.3s before trigger drag
            onLongPressed: {
                dndItem.canDrag = true
                iconLoader.scale = 1.2 // scale to 1.2 to prompt user can drag now
            }
            onTapped: {
                if (root.icons) {
                    root.folderClicked()
                } else {
                    root.itemClicked()
                }
            }
            onPressedChanged: {
                if (!pressed && !dragHandler.active) {
                    iconLoader.scale = 1.0
                    disableDragTimer.start()
                }
            }
            onCanceled: {
                iconLoader.scale = 1.0
                disableDragTimer.start()
            }
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: {
                root.menuTriggered()
            }
        }
    }
    background: DebugBounding { }

    Timer {
        id: disableDragTimer
        interval: 100
        repeat: false
        onTriggered: {
            dndItem.canDrag = false;
        }
    }

    DragHandler {
        id: dragHandler
        enabled: root.dndEnabled
        acceptedButtons: Qt.LeftButton
        onActiveChanged: {
            if (active) {
                disableDragTimer.stop()
                if (dndItem.canDrag) {
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
