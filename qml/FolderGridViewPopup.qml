// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import org.deepin.launchpad.windowed 1.0 as Windowed

Popup {
    id: root

    property alias currentFolderId: folderLoader.currentFolderId
    property alias folderName: folderLoader.folderName
    property var folderNameFont: DTK.fontManager.t3
    readonly property bool isWindowedMode: LauncherController.currentFrame === "WindowedFrame"

    modal: true
    focus: true
    // visible: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property var backgroundAlpha: 0.15
    property int cs: 110 // * 5 / 4
    // anchors.centerIn: parent // seems dtkdeclarative's Popup doesn't have anchors.centerIn

    width: cs * 4 + 20 /* padding */
    height: cs * 3 + 130 /* title height*/
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    onAboutToHide: {
        // reset folder view
        folderLoader.currentFolderId = -1
    }

    Loader {
        id: folderLoader

        property string folderName: "Sample Text"
        property int currentFolderId: -1

        active: currentFolderId !== -1
        anchors.fill: parent

        sourceComponent: Control {
            // Ensure drop won't fallthough the Popup.
            background: DropArea {
                anchors.fill: parent
                onDropped: {
                    let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                    dropOnPage(dragId, "internal/folders/" + folderLoader.currentFolderId, folderPagesView.currentIndex)
                }
            }

            contentItem: ColumnLayout {
                spacing: isWindowedMode ? 0 : 5
                anchors.fill: parent

                Item {
                    visible: !isWindowedMode
                    Layout.preferredHeight: 5
                }

                SystemPalette { id: folderTextPalette }
                TextInput {
                    Layout.fillWidth: true

                    font: folderNameFont
                    horizontalAlignment: Text.AlignHCenter
                    text: folderLoader.folderName
                    color: folderTextPalette.text
                    onEditingFinished: {
                        ItemArrangementProxyModel.updateFolderName(folderLoader.currentFolderId, text);
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"

                    SwipeView {
                        id: folderPagesView
                        clip: true

                        anchors.fill: parent

                        currentIndex: folderPageIndicator.currentIndex

                        Repeater {
                            model: ItemArrangementProxyModel.pageCount(folderLoader.currentFolderId) // FIXME: should be a property?

                            Loader {
                                active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem
                                id: folderGridViewLoader
                                objectName: "Folder GridView Loader"

                                sourceComponent: Rectangle {
                                    anchors.fill: parent
                                    color: "transparent"

                                    MultipageSortFilterProxyModel {
                                        id: folderProxyModel
                                        sourceModel: ItemArrangementProxyModel
                                        pageId: modelData
                                        folderId: folderLoader.currentFolderId
                                    }

                                    //gridViewContainer
                                    Loader {
                                        id: gridViewContainerLoader
                                        anchors.fill: parent

                                        sourceComponent: isWindowedMode ? listViewGridViewContainer : fullScreenGridViewContainer
                                    }

                                    Component {
                                        id: fullScreenGridViewContainer
                                        GridViewContainer {
                                            id: folderGridViewContainer
                                            anchors.fill: parent
                                            rows: 3
                                            columns: 4
                                            model: folderProxyModel
                                            padding: 10
                                            interactive: false
                                            focus: true
                                            activeGridViewFocusOnTab: folderGridViewLoader.SwipeView.isCurrentItem
                                            itemMove: Transition { NumberAnimation { properties: "x,y"; duration: 250 } }
                                            delegate: DelegateDropArea {
                                                width: folderGridViewContainer.cellWidth
                                                height: folderGridViewContainer.cellHeight
                                            }
                                        }
                                    }

                                    Component {
                                        id: listViewGridViewContainer
                                        Windowed.GridViewContainer {
                                            id: folderGridViewContainer
                                            anchors.fill: parent
                                            rows: 3
                                            columns: 4
                                            model: folderProxyModel
                                            paddingRows: 6
                                            paddingColumns: 2
                                            interactive: false
                                            focus: true
                                            activeGridViewFocusOnTab: folderGridViewLoader.SwipeView.isCurrentItem
                                            delegate: DelegateDropArea {
                                                width: folderGridViewContainer.cellWidth
                                                height: folderGridViewContainer.cellHeight
                                            }
                                        }
                                    }

                                    component DelegateDropArea: DropArea {
                                        onDropped: function(drop) {
                                            let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                                            if (dragId === model.desktopId) {
                                                return
                                            }

                                            let op = 1 // DndPrepend = -1, DndJoin = 0, DndAppend = 1
                                            let sideOpPadding = width / 2
                                            if (drop.x < sideOpPadding) {
                                                op = -1
                                            }

                                            dropOnItem(dragId, model.desktopId, op)
                                        }

                                        IconItemDelegate {
                                            anchors.fill: parent
                                            dndEnabled: true
                                            displayFont: isWindowedMode ? DTK.fontManager.t9 : DTK.fontManager.t8
                                            Drag.mimeData: {
                                                "text/x-dde-launcher-dnd-desktopId": model.desktopId
                                            }
                                            visible: dndItem.currentlyDraggedId !== model.desktopId
                                            iconSource: iconName

                                            padding: 5
                                            onItemClicked: {
                                                launchApp(desktopId)
                                            }
                                            onMenuTriggered: {
                                                showContextMenu(this, model, false, false, true)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                PageIndicator {
                    Layout.alignment: Qt.AlignHCenter

                    id: folderPageIndicator
                    implicitHeight: isWindowedMode ? 13 : folderPageIndicator.implicitWidth
                    count: folderPagesView.count
                    currentIndex: folderPagesView.currentIndex
                    interactive: true
                }
            }
        }
    }
    background: InWindowBlur {
        id: blur
        implicitWidth: DS.Style.popup.width
        implicitHeight: DS.Style.popup.height
        radius: DS.Style.popup.radius
        offscreen: true
        ItemViewport {
            anchors.fill: parent
            fixed: true
            sourceItem: blur.content
            radius: DS.Style.popup.radius
            hideSource: false
        }

        Rectangle {
            anchors.fill: parent
            radius: DS.Style.popup.radius
            color: Qt.rgba(255.0, 255.0, 255.0, backgroundAlpha)
        }
    }
}