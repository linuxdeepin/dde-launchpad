// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//import QtCore
import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0
import org.deepin.vendored 1.0

import org.deepin.launchpad 1.0

Control {
    visible: true
    anchors.fill: parent
    focus: true

    leftPadding: (DesktopIntegration.dockPosition === Qt.LeftArrow ? DesktopIntegration.dockGeometry.width : 0) + 20
    rightPadding: (DesktopIntegration.dockPosition === Qt.RightArrow ? DesktopIntegration.dockGeometry.width : 0) + 20
    topPadding: (DesktopIntegration.dockPosition === Qt.UpArrow ? DesktopIntegration.dockGeometry.height : 0) + 20
    bottomPadding: (DesktopIntegration.dockPosition === Qt.DownArrow ? DesktopIntegration.dockGeometry.height : 0) + 20

    Timer {
        id: flipPageDelay
        interval: 400
        repeat: false
    }

    background: Image {
        source: DesktopIntegration.backgroundUrl

        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.25)

            MouseArea {
                anchors.fill: parent
                scrollGestureEnabled: false
                onClicked: {
                    if (!DebugHelper.avoidHideWindow) {
                        LauncherController.visible = false
                    }
                }
                // TODO: this might not be the correct way to handle wheel
                onWheel: {
                    if (flipPageDelay.running) return
                    let xDelta = wheel.angleDelta.x / 8
                    let yDelta = wheel.angleDelta.y / 8
                    let toPage = 0; // -1 prev, +1 next, 0 don't change
                    if (yDelta !== 0) {
                        toPage = (yDelta > 0) ? -1 : 1
                    } else if (xDelta !== 0) {
                        toPage = (xDelta > 0) ? 1 : -1
                    }
                    let curPage = indicator.currentIndex + toPage
                    if (curPage >= 0 && curPage < indicator.count) {
                        flipPageDelay.start()
                        indicator.currentIndex = curPage
                    }
                }
            }
        }
    }

    contentItem: ColumnLayout {

        Control {
            Layout.fillWidth: true
            Layout.fillHeight: false

            contentItem: Rectangle {
                id: fullscreenHeader
                implicitHeight: exitFullscreenBtn.height
                color: "transparent"

                IconButton {
                    id: exitFullscreenBtn

                    Accessible.name: "Exit fullscreen"

                    anchors.right: fullscreenHeader.right

                    ColorSelector.family: Palette.CrystalColor

                    icon.name: "exit_fullscreen"
                    onClicked: {
                        LauncherController.currentFrame = "WindowedFrame"
                    }
                }

                SearchEdit {
                    id: searchEdit

                    anchors.centerIn: parent
                    width: (parent.width / 3) > 400 ? 400 : (parent.width / 3)

                    placeholder: qsTranslate("WindowedFrame", "Search")
                    onTextChanged: {
//                        console.log(text)
                        SearchFilterProxyModel.setFilterRegularExpression(text)
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            SwipeView {
                id: pages

                anchors.fill: parent
                visible: searchEdit.text === ""

                currentIndex: indicator.currentIndex

                Repeater {
                    model: MultipageProxyModel.pageCount(0) // FIXME: should be a property?

                    Loader {
                        active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem
                        id: gridViewLoader
                        objectName: "Main GridView Loader"

                        sourceComponent: Rectangle {
                            color: "transparent"

                            KSortFilterProxyModel {
                                id: proxyModel
                                sourceModel: MultipageProxyModel
                                filterRowCallback: (source_row, source_parent) => {
                                    var index = sourceModel.index(source_row, 0, source_parent);
                                    return sourceModel.data(index, MultipageProxyModel.PageRole) === modelData &&
                                           sourceModel.data(index, MultipageProxyModel.FolderIdNumberRole) === 0;
                                }
                            }

                            GridViewContainer {
                                id: gridViewContainer
                                anchors.fill: parent
                                rows: 4
                                columns: 7
                                model: proxyModel
                                padding: 10
                                interactive: false
                                focus: true
                                activeGridViewFocusOnTab: gridViewLoader.SwipeView.isCurrentItem
                                delegate: IconItemDelegate {
                                    iconSource: "image://app-icon/" + iconName
                                    width: gridViewContainer.cellSize
                                    height: gridViewContainer.cellSize
                                    icons: folderIcons
                                    padding: 5
                                    onItemClicked: {
                                        launchApp(desktopId)
                                    }
                                    onFolderClicked: {
                                        let idStr = model.desktopId
                                        let idNum = Number(idStr.replace("internal/folders/", ""))
                                        folderLoader.currentFolderId = idNum
                                        folderGridViewPopup.open()
                                        folderLoader.folderName = model.display
                                        console.log("open folder id:" + idNum)
                                    }
                                    onMenuTriggered: {
                                        showContextMenu(this, model, folderIcons, false, true)
                                    }
                                }
                            }
                        }

                        // Since SwipeView will catch the mouse click event so we need to also do it here...
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (!DebugHelper.avoidHideWindow) {
                                    LauncherController.visible = false
                                }
                            }
                        }
                    }
                }
            }

            DelegateModel {
                id: delegateSearchResultModel
                model: SearchFilterProxyModel
                delegate: IconItemDelegate {
                    iconSource: "image://app-icon/" + iconName
                    width: searchResultGridViewContainer.cellSize
                    height: searchResultGridViewContainer.cellSize
                    padding: 5
                    onItemClicked: {
                        launchApp(desktopId)
                    }
                    onMenuTriggered: {
                        showContextMenu(this, model, false, false, true)
                    }
                }
            }

            GridViewContainer {
                id: searchResultGridViewContainer

                anchors.fill: parent
                visible: searchEdit.text !== ""
                activeFocusOnTab: visible && gridViewFocus
                focus: true

                rows: 4
                columns: 7
                placeholderIcon: "search_no_result"
                placeholderText: qsTranslate("WindowedFrame", "No search results")
                placeholderIconSize: 256
                model: delegateSearchResultModel
                padding: 10
                interactive: false
            }
        }


        PageIndicator {
            Layout.alignment: Qt.AlignHCenter

            id: indicator

//            visible: pages.visible
            count: searchResultGridViewContainer.visible ? 1 : pages.count
            currentIndex: searchResultGridViewContainer.visible ? 1 : pages.currentIndex
            interactive: true
        }
    }

    Popup {
        id: folderGridViewPopup

        focus: true
//        visible: true

        property int cs: searchResultGridViewContainer.cellSize // * 5 / 4
//        anchors.centerIn: parent // seems dtkdeclarative's Popup doesn't have anchors.centerIn

        width: cs * 4 + 20 /* padding */
        height: cs * 3 + 130 /* title height*/
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        modal: true

        Loader {
            id: folderLoader

            property string folderName: "Sample Text"
            property int currentFolderId: 0

            active: currentFolderId !== 0
            anchors.fill: parent

            sourceComponent: ColumnLayout {
                spacing: 5
                anchors.fill: parent

                Item {
                    Layout.preferredHeight: 5
                }

                Label {
                    Layout.fillWidth: true

                    font: DTK.fontManager.t3
                    horizontalAlignment: Text.AlignHCenter
                    text: folderLoader.folderName
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"

                    SwipeView {
                        id: folderPagesView

                        anchors.fill: parent

                        currentIndex: folderPageIndicator.currentIndex

                        Repeater {
                            model: MultipageProxyModel.pageCount(folderLoader.currentFolderId) // FIXME: should be a property?

                            Loader {
                                active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem
                                id: folderGridViewLoader
                                objectName: "Folder GridView Loader"

                                sourceComponent: Rectangle {
                                    anchors.fill: parent
                                    color: "transparent"

                                    KSortFilterProxyModel {
                                        id: folderProxyModel
                                        sourceModel: MultipageProxyModel
                                        filterRowCallback: (source_row, source_parent) => {
                                            var index = sourceModel.index(source_row, 0, source_parent);
                                            return sourceModel.data(index, MultipageProxyModel.PageRole) === modelData &&
                                                   sourceModel.data(index, MultipageProxyModel.FolderIdNumberRole) === folderLoader.currentFolderId;
                                        }
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
                                        delegate: IconItemDelegate {
                                            iconSource: "image://app-icon/" + iconName
                                            width: folderGridViewContainer.cellSize
                                            height: folderGridViewContainer.cellSize
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

                    count: folderPagesView.count
                    currentIndex: folderPagesView.currentIndex
                    interactive: true
                }
            }
        }
    }

    Keys.onPressed: {
        if (searchEdit.focus === false && !searchEdit.text
                && event.modifiers === Qt.NoModifier
                && event.key >= Qt.Key_A && event.key <= Qt.Key_Z) {
            searchEdit.focus = true
            searchEdit.text = event.text
        }
    }

    Keys.onEscapePressed: {
        if (!DebugHelper.avoidHideWindow) {
            LauncherController.visible = false;
        }
    }
}
