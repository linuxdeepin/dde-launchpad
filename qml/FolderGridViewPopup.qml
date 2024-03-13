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

Popup {
    //id: folderGridViewPopup

    property alias currentFolderId: folderLoader.currentFolderId
    property alias folderName: folderLoader.folderName

    modal: true
    focus: true
    // visible: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property var backgroundAlph: 0.15
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
                spacing: 5
                anchors.fill: parent

                Item {
                    Layout.preferredHeight: 5
                }

                SystemPalette { id: folderTextPalette }
                TextInput {
                    Layout.fillWidth: true

                    font: DTK.fontManager.t3
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
                                        delegate: DropArea {
                                            width: folderGridViewContainer.cellWidth
                                            height: folderGridViewContainer.cellHeight
                                            onDropped: {
                                                let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                                                let op = 0
                                                let sideOpPadding = width / 4
                                                if (drop.x < sideOpPadding) {
                                                    op = -1
                                                } else if (drop.x > (width - sideOpPadding)) {
                                                    op = 1
                                                }
                                                dropOnItem(dragId, model.desktopId, op)
                                            }

                                            IconItemDelegate {
                                                anchors.fill: parent
                                                dndEnabled: true
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
                }

                PageIndicator {
                    Layout.alignment: Qt.AlignHCenter

                    id: folderPageIndicator

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
            color: Qt.rgba(255.0, 255.0, 255.0, backgroundAlph)
        }
    }
}