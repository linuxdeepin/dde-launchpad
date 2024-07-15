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

    signal folderClicked(string folderId, string folderName)

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

        onActiveFocusChanged: {
            if (activeFocus) {
                listView.currentIndex = 0
                listView.positionViewAtIndex(listView.currentIndex, 0)
            }
        }

        ScrollBar.vertical: ScrollBar { }

        model: freeSortProxyModel
        delegate: DropArea {
            width: listView.width
            height: itemDelegate.height
            keys: ["text/x-dde-launcher-dnd-desktopId"]

            property bool showDropIndicator: false
            property int op: 0 // DndPrepend = -1,DndJoin = 0, DndAppend = 1
            readonly property int indicatorDefaultHeight: 1

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
                    folderClicked(strFolderId, strFolderName)
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
                dropPositionCheck(drag.y, isDraggingFolder)
            }

            onEntered: {
                if (folderGridViewPopup.opened) {
                    folderGridViewPopup.close()
                }
            }

            onExited: {
                showDropIndicator = false;
            }

            onDropped: function(drop) {
                drop.accept()
                let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                showDropIndicator = false
                dropOnItem(dragId, model.desktopId, op)
            }

            Rectangle {
                id: dropIndicator
                x : 10
                width: bg.width
                height: indicatorDefaultHeight
                color: "lightgray"
                visible: showDropIndicator
                radius: height > indicatorDefaultHeight ? 8 : 1
                opacity: 0.5
            }

            ItemDelegate {
                id: itemDelegate
                text: model.display.startsWith("internal/category/") ? getCategoryName(model.display.substring(18)) : model.display
                checkable: false
                icon.name: itemType === ItemArrangementProxyModel.FolderItemType ? "folder" : iconName
                DciIcon.mode: DTK.NormalState
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                font: DTK.fontManager.t8
                palette.windowText: parent.palette.brightText
                visible: !Drag.active

                ToolTip.text: text
                ToolTip.delay: 1000
                ToolTip.visible: hovered && contentItem.implicitWidth > contentItem.width

                Drag.hotSpot.x: width / 3
                Drag.hotSpot.y: height / 2
                Drag.dragType: Drag.Automatic
                Drag.active: mouseArea.drag.active
                Drag.mimeData: {
                    "text/x-dde-launcher-dnd-desktopId": model.desktopId
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

    FreeSortProxyModel {
        id: freeSortProxyModel
        sourceModel: ItemArrangementProxyModel
    }
}
