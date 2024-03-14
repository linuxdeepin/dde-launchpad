// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import "."

Item {
    id: baseLayer
    objectName: "WindowedFrame-BaseLayer"

    visible: true
    focus: true

    // ----------- Drag and Drop related functions START -----------
    Label {
        property string currentlyDraggedId

        id: dndItem
        visible: DebugHelper.qtDebugEnabled
        text: "DnD DEBUG"

        Drag.onActiveChanged: {
            if (Drag.active) {
                text = "Dragging " + currentlyDraggedId
            } else {
                currentlyDraggedId = ""
            }
        }
    }

    function dropOnItem(dragId, dropId, op) {
        dndItem.text = "drag " + dragId + " onto " + dropId + " with " + op
        ItemArrangementProxyModel.commitDndOperation(dragId, dropId, op)
    }

    function dropOnPage(dragId, dropFolderId, pageNumber) {
        dndItem.text = "drag " + dragId + " into " + dropFolderId + " at page " + pageNumber
        ItemArrangementProxyModel.commitDndOperation(dragId, dropFolderId, ItemArrangementProxyModel.DndJoin, pageNumber)
    }
    // ----------- Drag and Drop related functions  END  -----------

    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        Layout.margins: 0

        RowLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Helper.windowed.topMargin

            SideBar {
                id: sideBar
            }

            Rectangle {
                Layout.preferredWidth: 2
                Layout.fillHeight: true
                color: this.palette.shadow
            }

            AppList {
                id: appList
                Layout.fillWidth: true
            }

            ColumnLayout {
                spacing: 0
                Layout.preferredWidth: 362
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: Helper.frequentlyUsed.rightMargin - 10
                Layout.margins: 0

                // Binding SearchFilterProxyModel with RecentlyInstalledProxyModel
                Binding {
                    target: SearchFilterProxyModel; property: "recentlyInstalledModel"
                    value: recentlyInstalledViewContainer.model
                }

                Label {
                    text: qsTr("My FrequentlyUseds")
                    font: LauncherController.boldFont(DTK.fontManager.t6)
                }

                Item {
                    Layout.alignment: Qt.AlignRight
                    Layout.topMargin: 10
                    Layout.preferredHeight: frequentlyUsedViewContainer.height
                    Layout.preferredWidth: frequentlyUsedViewContainer.width

                    GridViewContainer {
                        id: frequentlyUsedViewContainer

                        placeholderIcon: "search_no_result"
                        placeholderText: qsTr("No search results")
                        model: CountLimitProxyModel {
                            sourceModel: SearchFilterProxyModel
                            maxRowCount: recentlyInstalledView.visible ? 12 : 16
                        }
                        delegate: IconItemDelegate {
                            width: frequentlyUsedViewContainer.cellWidth
                            height: frequentlyUsedViewContainer.cellHeight
                            iconSource: iconName
                            onItemClicked: {
                                launchApp(desktopId)
                            }
                            onMenuTriggered: {
                                showContextMenu(this, model, false, false, false)
                            }
                        }

                        activeFocusOnTab: gridViewFocus

                        MouseArea {
                            enabled: recentlyInstalledView.visible && (currentIndex !== -1)

                            property int currentIndex: -1
                            property int modelType: -1 // 1: fav, 2: all

                            anchors.fill: parent
                            onPressAndHold: {
                                if (index !== -1) {
                                    currentIndex = index
                                }
                            }
                            onReleased: currentIndex = -1
                        }
                    }
                }


                Label {
                    Layout.topMargin: 10 - 10
                    visible: recentlyInstalledView.visible
                    text: qsTr("Recently Installed")
                    font: LauncherController.boldFont(DTK.fontManager.t6)
                }

                Item {
                    id: recentlyInstalledView
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredHeight: recentlyInstalledViewContainer.height
                    Layout.preferredWidth: recentlyInstalledViewContainer.width
                    Layout.topMargin: 6
                    visible: searchEdit.text === "" && recentlyInstalledViewContainer.count > 0

                    GridViewContainer {
                        id: recentlyInstalledViewContainer
                        model: CountLimitProxyModel {
                            sourceModel: RecentlyInstalledProxyModel
                            maxRowCount: 4
                        }
                        delegate: IconItemDelegate {
                            iconSource: iconName
                            width: recentlyInstalledViewContainer.cellWidth
                            height: recentlyInstalledViewContainer.cellHeight
                            onItemClicked: {
                                launchApp(desktopId)
                            }
                            onMenuTriggered: {
                                showContextMenu(this, model, false, true, false)
                            }
                        }
                        activeFocusOnTab: visible && gridViewFocus
                    }
                }
                Item {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                }
            }
        }

        Rectangle {
            Layout.preferredHeight: 2
            Layout.fillWidth: true
            color: this.palette.shadow
        }

        RowLayout {
            // Layout.preferredHeight: 20
            Layout.fillWidth: true
            Layout.margins: 10

            ToolButton {
                icon.name: "shutdown"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                ToolTip.text: qsTr("Power")
                onClicked: {
                    DesktopIntegration.openShutdownScreen();
                }
            }

            SearchEdit {
                id: searchEdit

                Layout.leftMargin: parent.width / 4
                Layout.rightMargin: parent.width / 4

                Layout.fillWidth: true
                placeholder: qsTr("Search")
                onTextChanged: {
                    console.log(text)
                    SearchFilterProxyModel.setFilterRegularExpression(text)
                    SearchFilterProxyModel.invalidate()
                }

                Palette {
                    id: actionPalette
                    normal {
                        common: ("#f7f7f7")
                        crystal: Qt.rgba(0, 0, 0, 0.1)
                    }
                    normalDark {
                        common: Qt.rgba(1, 1, 1, 0.1)
                        crystal: Qt.rgba(1, 1, 1, 0.1)
                    }
                    hovered {
                        common: ("#e1e1e1")
                        crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
                    }
                    pressed {
                        common: ("#bcc4d0")
                        crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
                    }
                }

                Palette {
                    id: nomalPalette
                    normal {
                        common: ("transparent")
                        crystal: Qt.rgba(0, 0, 0, 0.1)
                    }
                    normalDark {
                        common: ("transparent")
                        crystal: Qt.rgba(1, 1, 1, 0.1)
                    }
                    hovered {
                        common: ("#e1e1e1")
                        crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
                    }
                    pressed {
                        common: ("#bcc4d0")
                        crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
                    }
                }

                backgroundColor: searchEdit.editting ? actionPalette : nomalPalette
            }

            ToolButton {
                icon.name: "launcher_fullscreen"
                Accessible.name: "Fullscreen"
                onClicked: {
                    LauncherController.currentFrame = "FullscreenFrame"
                }
            }
        }
    }

    FolderGridViewPopup {
        id: folderGridViewPopup
        backgroundAlph: 0.8

        onVisibleChanged: {
            if (!visible) {
                baseLayer.opacity = 1
            }
        }
    }

    Keys.onPressed: {
        if (searchEdit.focus === false && !searchEdit.text && (event.text && !"\t ".includes(event.text))) {
            searchEdit.focus = true
            searchEdit.text = event.text
        }
    }

    Keys.onEscapePressed: {
        if (!DebugHelper.avoidHideWindow) {
            LauncherController.visible = false;
        }
    }

    Connections {
        target: LauncherController
        function onVisibleChanged() {
            // only do these clean-up steps on launcher get hide
            if (LauncherController.visible) return

            // clear searchEdit text
            searchEdit.text = ""
            // reset(remove) keyboard focus
            baseLayer.focus = true
            // reset scroll area position
            appList.positionViewAtBeginning()
            frequentlyUsedViewContainer.positionViewAtBeginning()
            recentlyInstalledViewContainer.positionViewAtBeginning()
        }
    }

    Connections {
        target: sideBar
        function onSwitchToFreeSort(isFreeSort) {
            appList.switchToFreeSort(isFreeSort)
        }
    }

    Connections {
        target: appList
        function onFreeSortViewFolderClicked(folderId, folderName) {
            folderGridViewPopup.currentFolderId = folderId
            folderGridViewPopup.folderName = folderName
            folderGridViewPopup.open()

            baseLayer.opacity = 0.1
        }
    }
}
