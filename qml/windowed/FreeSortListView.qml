// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15 as QQC2
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Item {
    id: root

    signal folderClicked(string folderId, string folderName)

    function positionViewAtBeginning() {
        listView.positionViewAtBeginning()
    }

    ListView {
        id: listView
        anchors.fill: parent
        activeFocusOnTab: true
        highlightFollowsCurrentItem: true

        clip: true
        focus: true
        highlight: Item {
            SystemPalette { id: highlightPalette }
            FocusBoxBorder {
                anchors {
                    fill: parent
                    margins: 4
                }
                radius: 4
                color: highlightPalette.highlight
                visible: listView.activeFocus
            }
        }

        ScrollBar.vertical: ScrollBar { }

        model: freeSortProxyModel
        delegate: DropArea {
            width: listView.width
            height: itemDelegate.height
            onEntered: {
                if (folderGridViewPopup.opened) {
                    folderGridViewPopup.close()
                }
            }
            onDropped: function(drop) {
                drop.accept()
                let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                let op = 0 // DndPrepend = -1,DndJoin = 0, DndAppend = 1
                let sideOpPadding = height / 3
                if (drop.y < sideOpPadding) {
                    op = -1
                } else if (drop.y > (height - sideOpPadding)) {
                    op = 1
                }
                dropOnItem(dragId, model.desktopId, op)
            }

            ItemDelegate {
                id: itemDelegate
                text: model.display.startsWith("internal/category/") ? getCategoryName(model.display.substring(18)) : model.display
                checkable: false
                icon.name: iconName === undefined ? "folder-icon" : iconName
                anchors.fill: parent
                font: DTK.fontManager.t8
                ColorSelector.family: Palette.CrystalColor
                visible: !Drag.active

                Drag.hotSpot.x: width / 3
                Drag.hotSpot.y: height / 2
                Drag.dragType: Drag.Automatic
                //Drag.source: itemDelegate
                Drag.active: dragHandler.active
                Drag.mimeData: {
                    "text/x-dde-launcher-dnd-desktopId": model.desktopId
                }

                TapHandler {
                    acceptedButtons: Qt.RightButton
                    onTapped: {
                        showContextMenu(itemDelegate, model, false, false, false)
                    }
                }

                Keys.onReturnPressed: {
                    launchApp(desktopId)
                }

                TapHandler {
                    onTapped: {
                        if (!iconName) {
                            console.log("freesort view folder clicked:", desktopId);
                            let idStr = model.desktopId
                            let strFolderId = Number(idStr.replace("internal/folders/", ""))
                            let strFolderName = model.display.startsWith("internal/category/") ? getCategoryName(model.display.substring(18)) : model.display
                            folderClicked(strFolderId, strFolderName)
                        } else {
                            launchApp(desktopId)
                        }
                    }
                }

                DragHandler {
                    id: dragHandler
                    onActiveChanged: {
                        if (active) {
                            bg.visible = true
                            itemDelegate.grabToImage(function(result) {
                                parent.Drag.imageSource = result.url;
                            })
                        } else {
                            bg.visible = false
                        }
                    }
                }

                background: BoxPanel {
                    id: bg
                    visible: ColorSelector.controlState === DTK.HoveredState
                    outsideBorderColor: null
                }
            }
        }
    }

    FreeSortProxyModel {
        id: freeSortProxyModel
        sourceModel: ItemArrangementProxyModel
    }
}