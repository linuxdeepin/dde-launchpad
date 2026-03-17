// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
    property bool isDragHover: false
    readonly property bool isWindowedMode: LauncherController.currentFrame === "WindowedFrame"
    property alias displayFont: iconItemLabel.font
    property real iconScaleFactor: 1.0
    property bool iconIntroAnimRunning: false

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
        hoverEnabled: !root.iconIntroAnimRunning
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
                id: iconContainer
                width: parent.width / 2
                height: width
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    id: dragAndfolderBackground
                    visible: root.icons !== undefined || (root.isDragHover && !isWindowedMode)
                    opacity: root.icons !== undefined || (root.isDragHover && !isWindowedMode) ? 1 : 0
                    scale:  (root.isDragHover && !isWindowedMode) ? 1.2 : 1
                    color: "#26FFFFFF"
                    Behavior on opacity {
                        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
                    }
                    Behavior on scale {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }
                    anchors.fill: parent
                    radius: 12

                    NumberAnimation on scale {
                        id: ininAni
                        running: false
                        from: 1.2
                        to: 1
                        duration: 200
                        easing.type: Easing.OutCubic
                    }

                    Component.onCompleted: {
                        if (root.icons !== undefined && dndItem.mergeAnimTargetIcon && dndItem.mergeAnimTargetIcon2) {
                            ininAni.start()
                        }
                    }
                }

                Loader {
                    id: iconLoader
                    anchors.fill: parent
                    asynchronous: true
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
                                dndItem.currentlyDraggedIconName = root.iconSource
                                dndItem.Drag.hotSpot = target.Drag.hotSpot
                                dndItem.Drag.mimeData = target.Drag.mimeData
                                dndItem.mergeSize = Math.min(iconLoader.width, iconLoader.height)

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

                    Item {
                        id: iconItem
                        anchors.fill: parent
                        property real maxIconCount: 2
                        property real spacing: 8
                        property real itemWidth: (width - ((maxIconCount + 1) * spacing)) / 2
                        property real itemHeight: (height - ((maxIconCount + 1) * spacing)) / maxIconCount

                        function getItemX(index) {
                            let col = index % maxIconCount
                            let ItemX = (col + 1) * spacing + col * itemWidth

                            return ItemX
                        }

                        function getItemY(index) {
                            let row = Math.floor(index / maxIconCount)
                            let ItemY = (row + 1) * spacing + row * itemHeight
                            return ItemY
                        }
                        Repeater {
                            model: icons

                            DciIcon {
                                id: folderIcon
                                x: iconItem.getItemX(index)
                                y: iconItem.getItemY(index)

                                width: iconItem.itemWidth
                                height: iconItem.itemHeight

                                name: modelData
                                sourceSize: Qt.size(root.maxIconSizeInFolder, root.maxIconSizeInFolder)
                                scale: (itemWidth / root.maxIconSizeInFolder) * root.iconScaleFactor

                                property real introScale: 1.0

                                palette: DTK.makeIconPalette(root.palette)
                                theme: ApplicationHelper.DarkType

                                // 位移动画属性
                                property real iconCenterX: 0
                                property real iconCenterY: 0
                                ParallelAnimation {
                                    id: iconIntroAnim
                                    onStarted: root.iconIntroAnimRunning = true

                                    NumberAnimation {
                                        target: folderIcon
                                        property: "scale"
                                        from: folderIcon.introScale
                                        to: (itemWidth / root.maxIconSizeInFolder) * root.iconScaleFactor
                                        duration: 600
                                        easing.type: Easing.OutExpo
                                    }
                                    NumberAnimation {
                                        target: folderIcon
                                        property: "x"
                                        from: folderIcon.iconCenterX; to: iconItem.getItemX(index)
                                        duration: 800
                                        easing.type: Easing.OutExpo
                                    }
                                    NumberAnimation {
                                        target: folderIcon
                                        property: "y"
                                        from: folderIcon.iconCenterY; to: iconItem.getItemY(index)
                                        duration: 800
                                        easing.type: Easing.OutExpo
                                    }

                                    onFinished: {
                                        root.iconIntroAnimRunning = false
                                        dndItem.mergeAnimPending = false
                                        dndItem.mergeAnimTargetIcon = ""
                                        dndItem.mergeAnimTargetIcon2 = ""
                                    }
                                }

                                Component.onCompleted: {
                                    if (dndItem.mergeAnimPending
                                        && modelData === dndItem.mergeAnimTargetIcon) {
                                        folderIcon.visible = false
                                        Qt.callLater(function() {
                                            let localPos = iconItem.mapFromItem(null,
                                                dndItem.mergeAnimStartX, dndItem.mergeAnimStartY)
                                            folderIcon.iconCenterX = localPos.x - folderIcon.width / 2
                                            folderIcon.iconCenterY = localPos.y - folderIcon.height / 2
                                            folderIcon.introScale = (iconContainer.width / root.maxIconSizeInFolder) * root.iconScaleFactor
                                            folderIcon.visible = true
                                            iconIntroAnim.start()
                                        })
                                    } else if (dndItem.mergeAnimPending
                                        && modelData === dndItem.mergeAnimTargetIcon2) {
                                        Qt.callLater(function() {
                                            folderIcon.iconCenterX = iconContainer.width / 2 - folderIcon.width / 2
                                            folderIcon.iconCenterY = iconContainer.height / 2 - folderIcon.height / 2
                                            folderIcon.introScale = (iconContainer.width / root.maxIconSizeInFolder) * root.iconScaleFactor
                                            iconIntroAnim.start()
                                        })
                                    }
                                }
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

                Component {
                    id: imageComponent

                    DciIcon {
                        objectName: "appIcon"
                        anchors.fill: parent
                        name: iconSource
                        sourceSize: Qt.size(root.maxIconSize, root.maxIconSize)
                        scale: (iconContainer.width / root.maxIconSize) * root.iconScaleFactor
                        palette: DTK.makeIconPalette(root.palette)
                        theme: ApplicationHelper.DarkType
                        fillMode: Image.PreserveAspectFit
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
                property bool isNewlyInstalled: model.lastLaunchedTime === 0 && model.installedTime !== 0
                id: iconItemLabel
                visible: !root.isDragHover
                text: isNewlyInstalled ? ("<font color='#669DFF' size='1' style='text-shadow: 0 0 1px rgba(255,255,255,0.1)'>●</font>&nbsp;&nbsp;" + root.text) : root.text
                textFormat: isNewlyInstalled ? Text.StyledText : Text.PlainText
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
