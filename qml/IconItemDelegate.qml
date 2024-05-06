// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0
import org.deepin.dtk.private 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

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
                    anchors.fill: parent
                    sourceComponent: root.icons !== undefined ? folderComponent : imageComponent
                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onPressedChanged: {
                            if (pressed) {
                                root.Drag.hotSpot = point.pressPosition
                                iconLoader.grabToImage(function(result) {
                                    root.Drag.imageSource = result.url;
                                })
                            }
                        }
                    }
                    DragHandler {
                        id: dragHandler
                        target: root
                        acceptedButtons: Qt.LeftButton
                        enabled: root.dndEnabled
                        onActiveChanged: {
                            if (active) {
                                // We switch to use the `dndItem` to handle Drag event since that one will always exists.
                                // If we use the current item, then if the item that provides the drag attached property
                                // get destoryed (e.g. switch page or folder close caused destory), dropping at that moment
                                // will cause a crash.

                                // Item will be hidden by checking the dndItem.currentlyDraggedId property. We assign the value
                                // to that property here
                                dndItem.currentlyDraggedId = root.Drag.mimeData["text/x-dde-launcher-dnd-desktopId"]
                                // TODO: This way we couldn't give it an image size hint,
                                dndItem.Drag.imageSource = root.Drag.imageSource
                                dndItem.Drag.hotSpot = root.Drag.hotSpot
                                Qt.callLater(function() {
                                    dndItem.Drag.mimeData = root.Drag.mimeData
                                    dndItem.Drag.active = true
                                    dndItem.Drag.startDrag()
                                })
                            }
                        }
                    }
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
            insideBorderColor: isWindowedMode ? null : DS.Style.button.insideBorder

            property D.Palette background: D.Palette {
                normal {
                    common: Qt.rgba(0, 0, 0, 0.1)
                    crystal: Qt.rgba(0, 0, 0, 0.1)
                }
                normalDark {
                    common: Qt.rgba(1, 1, 1, 0.1)
                    crystal: Qt.rgba(1, 1, 1, 0.1)
                }
                hovered {
                    common: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.1)
                    crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.1)
                }
            }
            color1: isWindowedMode ? background : DS.Style.button.background1
            color2: isWindowedMode ? background : DS.Style.button.background2
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
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
    }
    background: DebugBounding { }

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
