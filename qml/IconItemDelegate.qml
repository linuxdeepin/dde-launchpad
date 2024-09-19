// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import org.deepin.dtk 1.0
import org.deepin.dtk.private 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import 'windowed'

Control {
    id: root

    property var icons: undefined

    // TODO: When DciIcon changes the sourceSize, the icon will flash, It may be a bug of dciicon or qt?
    // So we give the max sourceSize and use scale to solve it.
    property int maxIconSize: 128
    property int maxIconSizeInFolder: 64
    readonly property string text: display.startsWith("internal/category/") ? getCategoryName(display.substring(18)) : display

    property string iconSource
    property bool dndEnabled: false
    readonly property bool isWindowedMode: LauncherController.currentFrame === "WindowedFrame"
    property alias displayFont: iconItemLabel.font

    Accessible.name: iconItemLabel.text

    signal folderClicked()
    signal itemClicked()
    signal menuTriggered()

    Drag.dragType: Drag.Automatic

    states: State {
        name: "dragged";
        when: dragHandler.active
        // FIXME: When dragging finished, the position of the item is changed for unknown reason,
        //        so we use the state to reset the x and y here.
        PropertyChanges {
            target: dragHandler.target
            x: x
            y: y
        }
    }

    contentItem: Button {
        focusPolicy: Qt.NoFocus
        ColorSelector.pressed: false
        ColorSelector.family: D.Palette.CrystalColor
        flat: true
        contentItem: Column {
            anchors.fill: parent

            Item {
                // actually just a top padding
                width: root.width
                height: isWindowedMode ? 7 : root.height / 9
            }

            Item {
                width: parent.width / 2
                height: width
                anchors.horizontalCenter: parent.horizontalCenter

                Loader {
                    id: iconLoader
                    anchors.fill: parent
                    sourceComponent: root.icons !== undefined ? folderComponent : imageComponent
                    DragHandler {
                        id: dragHandler
                        target: root
                        acceptedButtons: Qt.LeftButton
                        enabled: root.dndEnabled
                        dragThreshold: 1
                        onActiveChanged: {
                            if (active) {
                                // We switch to use the `dndItem` to handle Drag event since that one will always exists.
                                // If we use the current item, then if the item that provides the drag attached property
                                // get destoryed (e.g. switch page or folder close caused destory), dropping at that moment
                                // will cause a crash.

                                // Item will be hidden by checking the dndItem.currentlyDraggedId property. We assign the value
                                // to that property here
                                dndItem.currentlyDraggedId = target.Drag.mimeData["text/x-dde-launcher-dnd-desktopId"]
                                dndItem.Drag.hotSpot = target.Drag.hotSpot
                                dndItem.Drag.mimeData = target.Drag.mimeData

                                iconLoader.grabToImage(function(result) {
                                    dndItem.Drag.imageSource = result.url;
                                    dndItem.Drag.active = true
                                    dndItem.Drag.startDrag()
                                })
                            }
                        }
                    }
                }

                DciIcon {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom

                    name: "emblem_autostart"
                    visible: autoStart
                    sourceSize: Qt.size(16, 16)
                    palette: DTK.makeIconPalette(root.palette)
                    theme: ApplicationHelper.DarkType
                }

                Component {
                    id: folderComponent

                    Rectangle {
                        anchors.fill: parent
                        color: "#26FFFFFF"
                        radius: 12

                        GridLayout {
                            anchors.fill: parent
                            rows: 2
                            columns: 2
                            anchors.margins: 8
                            columnSpacing: 8
                            rowSpacing: 8

                            Repeater {
                                model: icons

                                DciIcon {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                                    name: modelData
                                    sourceSize: Qt.size(root.maxIconSizeInFolder, root.maxIconSizeInFolder)
                                    scale: parent.width / 2 / root.maxIconSizeInFolder
                                    palette: DTK.makeIconPalette(root.palette)
                                    theme: ApplicationHelper.DarkType
                                }
                            }

                            Repeater {
                                model: 4 - icons.length

                                Item {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                                    width: parent.width / 2
                                    height: parent.height / 2
                                }
                            }
                        }
                    }
                }

                Component {
                    id: imageComponent

                    DciIcon {
                        objectName: "appIcon"
                        anchors.fill: parent
                        name: iconSource
                        sourceSize: Qt.size(root.maxIconSize, root.maxIconSize)
                        scale: parent.width / root.maxIconSize
                        palette: DTK.makeIconPalette(root.palette)
                        theme: ApplicationHelper.DarkType
                    }
                }
            }

            // as topMargin
            Item {
                width: 1
                height: isWindowedMode ? 4 : root.height / 10
            }

            Label {
                property bool singleRow: font.pixelSize > (isWindowedMode ? Helper.windowed.doubleRowMaxFontSize : Helper.fullscreen.doubleRowMaxFontSize)
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

            TapHandler {
                acceptedButtons: Qt.RightButton
                gesturePolicy: TapHandler.WithinBounds
                onTapped: {
                    root.menuTriggered()
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                gesturePolicy: TapHandler.WithinBounds
                onPressedChanged: {
                    if (pressed) {
                        root.Drag.hotSpot = mapToItem(iconLoader, point.pressPosition)
                    }
                }
                onTapped: {
                    if (model.itemType === ItemArrangementProxyModel.FolderItemType) {
                        root.folderClicked()
                    } else {
                        root.itemClicked()
                    }
                }
            }
        }
        ToolTip.text: root.text
        ToolTip.delay: 500
        ToolTip.visible: hovered && iconItemLabel.truncated
        background: ItemBackground {
            radius: isWindowedMode ? 8 : 18
            button: parent
        }
    }
    background: DebugBounding { }

    Keys.onSpacePressed: {
        if (model.itemType === ItemArrangementProxyModel.FolderItemType) {
            root.folderClicked()
        } else {
            root.itemClicked()
        }
    }

    Keys.onReturnPressed: {
        if (model.itemType === ItemArrangementProxyModel.FolderItemType) {
            root.folderClicked()
        } else {
            root.itemClicked()
        }
    }
}
