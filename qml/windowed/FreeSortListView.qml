// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15 as QQC2
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DStyle

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Item {
    id: root

    signal folderClicked(string folderId, string folderName, point triggerPosition)

    property Item keyTabTarget: listView

    onFocusChanged: () => {
        listView.focus = true
    }

    function positionViewAtBeginning() {
        listView.positionViewAtBeginning()
    }

    ListView {
        id: listView
        anchors.fill: parent
        highlightFollowsCurrentItem: true

        displaced: Transition { NumberAnimation { properties: "y"; duration: 150 } }
        move: displaced
        moveDisplaced: displaced

        clip: true
        highlight: Item {
            FocusBoxBorder {
                anchors {
                    fill: parent
                    margins: 4
                }
                radius: 4
                color: parent.palette.highlight
                visible: listView.activeFocus
            }
        }

        // End margin, see PMS-301113
        footer: Item {
            height: 10
        }

        onActiveFocusChanged: {
            if (activeFocus) {
                listView.currentIndex = 0
                listView.positionViewAtIndex(listView.currentIndex, 0)
            }
        }

        ScrollBar.vertical: ScrollBar { }

        Timer {
            id: listViewDragScroller

            property int scrollDelta: 0

            interval: 20
            repeat: true
            running: false
            onTriggered: function() {
                scroll()
            }

            function scroll() {
                if ((scrollDelta < 0 && !listView.atYBeginning) || (scrollDelta > 0 && !listView.atYEnd)) {
                    listView.contentY += scrollDelta
                }
            }

            function startScroll(delta) {
                scrollDelta = delta
                listViewDragScroller.start()
            }

            function stopScroll() {
                if (listViewDragScroller.running) {
                    scrollDelta = 0
                    listViewDragScroller.stop()
                }
            }
        }

        model: FreeSortProxyModel {
            id: freeSortProxyModel
            sourceModel: MultipageSortFilterProxyModel {
                filterOnlyMode: true
                sourceModel: ItemArrangementProxyModel
                pageId: -1
                folderId: 0
            }
            sortRole: ItemArrangementProxyModel.IndexInPageRole
            Component.onCompleted: {
                freeSortProxyModel.sort(0)
            }
        }
        delegate: DropArea {
            width: listView.width
            height: itemDelegate.height
            keys: ["text/x-dde-launcher-dnd-desktopId"]
            ListView.delayRemove: itemDelegate.Drag.active

            property bool showDropIndicator: false
            property int op: 0 // DndPrepend = -1,DndJoin = 0, DndAppend = 1
            readonly property int indicatorDefaultHeight: 1

            Timer {
                id: listDelegateDragApplyTimer
                interval: 500
                repeat: true
                running: false
                property string dragId: ""
                onTriggered: function() {
                    if (dragId === "") return
                    if (parent.op === 0) return
                    dropOnItem(dragId, model.desktopId, parent.op)
                    stopTimer()
                }

                function startTimer(id) {
                    dragId = id
                    start()
                }

                function stopTimer() {
                    dragId = ""
                    stop()
                }
            }

            function scrollViewWhenNeeded(mouseY) {
                let dragPosY = listView.mapFromItem(this, 0, mouseY).y
                if (dragPosY < (listView.height / 10) && !listView.atYBeginning) {
                    listViewDragScroller.startScroll(-10)
                } else if (dragPosY > (listView.height / 10 * 9) && !listView.atYEnd) {
                    listViewDragScroller.startScroll(10)
                } else {
                    listViewDragScroller.stopScroll()
                }
            }

            function dropPositionCheck(mouseY, isDraggingFolder) {
                let sideOpPadding = height / 3
                if (mouseY < sideOpPadding) {
                    dropIndicator.y = 0
                    dropIndicator.height = indicatorDefaultHeight
                    op = -1
                } else if (mouseY > (height - sideOpPadding)) {
                    dropIndicator.y = itemDelegate.height
                    dropIndicator.height = indicatorDefaultHeight
                    op = 1
                } else {
                    if (isDraggingFolder) {
                        dropIndicator.height = indicatorDefaultHeight
                        showDropIndicator = false
                    } else {
                        dropIndicator.y = 0
                        dropIndicator.height = itemDelegate.height
                    }

                    op = 0
                }
            }

            function launchItem() {
                if (itemType === ItemArrangementProxyModel.FolderItemType) {
                    console.log("freesort view folder clicked:", desktopId);
                    let idStr = model.desktopId
                    let strFolderId = Number(idStr.replace("internal/folders/", ""))
                    let strFolderName = model.display.startsWith("internal/category/") ? getCategoryName(model.display.substring(18)) : model.display
                    let offset = height / 2
                    folderClicked(strFolderId, strFolderName, mapToItem(listView, offset, offset))
                } else {
                    launchApp(desktopId)
                }
            }

            onPositionChanged: function(drag) {
                let dragId = drag.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                if (dragId === desktopId) {
                    return
                }

                let isDraggingFolder = false
                if (dragId.indexOf("internal/folders/") !== -1) {
                    isDraggingFolder = true
                }

                showDropIndicator = true
                scrollViewWhenNeeded(drag.y)
                dropPositionCheck(drag.y, isDraggingFolder)
            }

            onEntered: function(drag) {
                listDelegateDragApplyTimer.startTimer(drag.getDataAsString("text/x-dde-launcher-dnd-desktopId"))   
                if (folderGridViewPopup.opened) {
                    folderGridViewPopup.close()
                }
            }

            onExited: {
                showDropIndicator = false;
                listDelegateDragApplyTimer.stopTimer()
            }

            onDropped: function(drop) {
                listDelegateDragApplyTimer.stopTimer()
                drop.accept()
                let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                showDropIndicator = false
                dropOnItem(dragId, model.desktopId, op)
                listViewDragScroller.stopScroll()
            }

            Rectangle {
                id: dropIndicator
                x : 10
                width: bg.width
                height: indicatorDefaultHeight
                visible: showDropIndicator
                radius: height > indicatorDefaultHeight ? 8 : 1

                property Palette background: Helper.itemBackground
                color: dropIndicator.ColorSelector.background
            }

            ItemDelegate {
                id: itemDelegate
                text: model.display.startsWith("internal/category/") ? getCategoryName(model.display.substring(18)) : model.display
                checkable: false
                icon.name: itemType === ItemArrangementProxyModel.FolderItemType ? "folder" : iconName
                DciIcon.mode: DTK.NormalState
                DciIcon {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    name: "emblem_autostart"
                    visible: autoStart
                    sourceSize: Qt.size(12, 12)
                    palette: DTK.makeIconPalette(parent.palette)
                    theme: ApplicationHelper.DarkType
                    z: 1
                }
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: 10
                    rightMargin: 10
                }
                font: DTK.fontManager.t8
                palette.windowText: parent.palette.brightText
                visible: !Drag.active

                ToolTip.text: text
                ToolTip.delay: 500
                ToolTip.visible: hovered && contentItem.implicitWidth > contentItem.width

                Drag.hotSpot.x: width / 3
                Drag.hotSpot.y: height / 2
                Drag.dragType: Drag.Automatic
                Drag.active: mouseArea.drag.active
                Drag.mimeData: Helper.generateDragMimeData(model.desktopId)
                Drag.onActiveChanged: function() {
                    if (!Drag.active) {
                        listViewDragScroller.stopScroll()
                    }
                }

                background: ItemBackground {
                    id: bg
                    implicitWidth: DStyle.Style.itemDelegate.width
                    implicitHeight: Helper.windowed.listItemHeight
                    button: itemDelegate
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent

                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    drag.target: itemDelegate

                    onPressed: function (mouse) {
                        if (mouse.button === Qt.LeftButton) {
                            bg.visible = false
                            itemDelegate.grabToImage(function(result) {
                                itemDelegate.Drag.imageSource = result.url
                                bg.visible = Qt.binding(function() {
                                    return bg.ColorSelector.controlState === DTK.HoveredState
                                })
                            })
                        }
                    }

                    onClicked: function (mouse) {
                        if (mouse.button === Qt.RightButton) {
                            showContextMenu(itemDelegate, model, {
                                hideMoveToTopMenu: index === 0
                            })

                            baseLayer.focus = true
                        } else {
                            launchItem()
                        }
                    }
                }
            }

            Keys.onReturnPressed: launchItem()

            Keys.onSpacePressed: launchItem()
        }
    }
}
