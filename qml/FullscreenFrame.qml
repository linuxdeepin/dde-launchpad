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
import org.deepin.dtk.style 1.0 as DS

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Control {
    id: baseLayer
    visible: true
    anchors.fill: parent
    focus: true
    objectName: "FullscreenFrame-BaseLayer"

    readonly property bool isHorizontalDock: DesktopIntegration.dockPosition === Qt.UpArrow || DesktopIntegration.dockPosition === Qt.DownArrow
    readonly property int dockSpacing: (isHorizontalDock ? DesktopIntegration.dockGeometry.height : DesktopIntegration.dockGeometry.width) / Screen.devicePixelRatio

    leftPadding: (DesktopIntegration.dockPosition === Qt.LeftArrow ? dockSpacing : 0)
    rightPadding: (DesktopIntegration.dockPosition === Qt.RightArrow ? dockSpacing : 0)
    topPadding: (DesktopIntegration.dockPosition === Qt.UpArrow ? dockSpacing : 0) + 20
    bottomPadding: (DesktopIntegration.dockPosition === Qt.DownArrow ? dockSpacing : 0) + 20

    property Palette textColor: appTextColor
    palette.windowText: ColorSelector.textColor

    // ----------- Drag and Drop related functions START -----------
    Label {
        property string currentlyDraggedId

        signal dragEnded()

        id: dndItem
        visible: DebugHelper.qtDebugEnabled
        text: "DnD DEBUG"

        Drag.onActiveChanged: {
            if (Drag.active) {
                text = "Dragging " + currentlyDraggedId
            } else {
                currentlyDraggedId = ""
                dragEnded()
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

    function tryToRemoveEmptyPage() {
        ItemArrangementProxyModel.removeEmptyPage()
    }

    function decrementPageIndex() {
        if (pages.currentIndex === 0 && pages.count > 1) {
            // pages.setCurrentIndex(pages.count - 1)
        } else {
            pages.decrementCurrentIndex()
        }

        closeContextMenu()
    }

    function incrementPageIndex() {
        if (pages.currentIndex === pages.count - 1 && pages.count > 1) {
            // pages.setCurrentIndex(0)
        } else {
            pages.incrementCurrentIndex()
        }

        closeContextMenu()
    }

    Timer {
        id: flipPageDelay
        interval: 400
        repeat: false
    }

    background: Image {
        source: DesktopIntegration.backgroundUrl

        Rectangle {
            anchors.fill: parent
            color: folderGridViewPopup.visible ? Qt.rgba(0, 0, 0, 0.6) : Qt.rgba(0, 0, 0, 0.5)

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
                    if (toPage < 0) {
                        flipPageDelay.start()
                        if (!searchEdit.focus) { // reset keyboard focus when using mouse to flip page, but keep searchEdit focus
                            baseLayer.focus = true
                        }
                        decrementPageIndex()
                    } else if (toPage > 0) {
                        flipPageDelay.start()
                        if (!searchEdit.focus) { // reset keyboard focus when using mouse to flip page, but keep searchEdit focus
                            baseLayer.focus = true
                        }
                        incrementPageIndex()
                    }
                }
            }
        }
    }

    contentItem: ColumnLayout {

        Control {
            Layout.fillWidth: true
            Layout.fillHeight: false

            leftPadding: 20
            rightPadding: 20
            topPadding: 10

            contentItem: Rectangle {
                id: fullscreenHeader
                implicitHeight: exitFullscreenBtn.height
                color: "transparent"

                ToolButton {
                    id: exitFullscreenBtn

                    visible: (DesktopIntegration.environmentVariable("DDE_CURRENT_COMPOSITOR") !== "TreeLand")

                    Accessible.name: "Exit fullscreen"

                    anchors.right: fullscreenHeader.right

                    ColorSelector.family: Palette.CrystalColor

                    icon.name: "launcher_exit_fullscreen"
                    onClicked: {
                        LauncherController.currentFrame = "WindowedFrame"
                    }
                }

                PageIndicator {
                    id: indicator

                    opacity: folderGridViewPopup.visible ? 0.4 : 1
                    anchors.horizontalCenter: parent.horizontalCenter
        //            visible: pages.visible
                    count: searchResultGridViewContainer.visible ? 1 : pages.count
                    currentIndex: searchResultGridViewContainer.visible ? 1 : pages.currentIndex
                    interactive: true
                    delegate: Rectangle {
                        width: DS.Style.pageIndicator.width
                        height: width

                        radius: width / 2
                        color: indicator.enabled ? "white" : "gray"

                        opacity: index === indicator.currentIndex ? 1 : pressed ? 0.7 : 0.40

                        OutsideBoxBorder {
                            anchors.fill: parent
                            radius: parent.radius
                            color: Qt.rgba(0, 0, 0, 0.2)
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            DropArea {
                id: dropArea
                property int pageIntent: 0
                readonly property real paddingColumns: 0.5
                readonly property int horizontalPadding:  searchResultGridViewContainer.cellWidth * paddingColumns
                anchors.fill: parent

                property bool createdEmptyPage: false
                function checkDragMove() {
                    if (drag.x < horizontalPadding) {
                        pageIntent = -1
                    } else if (drag.x > (width - searchResultGridViewContainer.cellWidth)) {
                        let isLastPage = pages.currentIndex === pages.count - 1
                        if (isLastPage && dropArea.createdEmptyPage) {
                            return
                        }
                        pageIntent = 1
                    } else {
                        pageIntent = 0
                    }
                }

                keys: ["text/x-dde-launcher-dnd-desktopId"]
                onEntered: {
                    if (folderGridViewPopup.opened) {
                        folderGridViewPopup.close()
                    }
                }
                onPositionChanged: {
                    checkDragMove()
                }
                onDropped: (drop) => {
                    // drop over the left or right boundary of the page, do nothing
                    if (pageIntent !== 0) {
                        pageIntent = 0
                        return
                    }
                    // drop into current page
                    let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                    dropOnPage(dragId, "internal/folders/0", pages.currentIndex)
                    pageIntent = 0
                }
                onExited: {
                    pageIntent = 0
                }
                onPageIntentChanged: {
                    if (pageIntent !== 0) {
                        dndMovePageTimer.restart()
                    } else {
                        dndMovePageTimer.stop()
                    }
                }

                Timer {
                    id: dndMovePageTimer
                    interval: 1000
                    onTriggered: {
                        if (parent.pageIntent > 0) {
                            let isLastPage = pages.currentIndex === pages.count - 1
                            if (isLastPage && !dropArea.createdEmptyPage) {
                                let newPageIndex = ItemArrangementProxyModel.creatEmptyPage()
                                dropArea.createdEmptyPage = true
                                pages.setCurrentIndex(newPageIndex)
                                parent.pageIntent = 0
                                return
                            } else {
                                incrementPageIndex()
                            }
                        } else if (parent.pageIntent < 0) {
                            decrementPageIndex()
                        }

                        parent.pageIntent = 0
                        if (pages.currentIndex !== 0) {
                            parent.checkDragMove()
                        }
                    }
                }

                Connections {
                    target: dndItem
                    function onDragEnded() {
                        if (dropArea.createdEmptyPage) {
                            tryToRemoveEmptyPage()
                            dropArea.createdEmptyPage = false
                        }
                    }
                }
            }

            SwipeView {
                id: pages

                property int previousIndex: -1
                anchors.fill: parent
                visible: searchEdit.text === ""

                currentIndex: indicator.currentIndex

                // To ensure toplevelRepeater's model (page count) updated correctly
                // Caution! Don't put it directly under a Repeater{}, that will prevent Connections from working
                Connections {
                    target: ItemArrangementProxyModel
                    function onRowsInserted() {
                        toplevelRepeater.pageCount = ItemArrangementProxyModel.pageCount(0)
                    }
                    function onRowsRemoved() {
                        toplevelRepeater.pageCount = ItemArrangementProxyModel.pageCount(0)
                    }

                    function onTopLevelPageCountChanged() {
                        toplevelRepeater.pageCount = ItemArrangementProxyModel.pageCount(0)
                    }
                }

                Repeater {
                    id: toplevelRepeater
                    property int pageCount: ItemArrangementProxyModel.pageCount(0)
                    model: pageCount

                    Loader {
                        active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem
                        id: gridViewLoader
                        objectName: "Main GridView Loader"

                        property int viewIndex: index

                        sourceComponent: Rectangle {
                            color: "transparent"

                            property var grids: gridViewContainer

                            MultipageSortFilterProxyModel {
                                id: proxyModel
                                sourceModel: ItemArrangementProxyModel
                                pageId: modelData
                                folderId: 0
                            }

                            GridViewContainer {
                                id: gridViewContainer
                                objectName: "gridViewContainer"
                                anchors.fill: parent
                                rows: 4
                                columns: 8
                                paddingColumns: 0.5
                                model: proxyModel
                                padding: 10
                                interactive: false
                                focus: true
                                function checkPageSwitchState() {
                                    if (gridViewLoader.viewIndex !== pages.currentIndex)
                                        return
                                    if (pages.previousIndex === -1) {
                                        pages.previousIndex = pages.currentIndex
                                        return
                                    }
                                    if (pages.currentIndex + 1 === pages.previousIndex || (pages.previousIndex === 0 && pages.currentIndex === pages.count - 1))
                                        gridViewContainer.setPreviousPageSwitch(true)
                                    else
                                        gridViewContainer.setPreviousPageSwitch(false)
                                    pages.previousIndex = pages.currentIndex
                                }

                                Keys.onLeftPressed: function(event) {
                                    if (gridViewLoader.SwipeView.index === 0 && toplevelRepeater.pageCount > 1) {
                                        // is the 1st page, go to last page
                                        pages.setCurrentIndex(toplevelRepeater.pageCount - 1)
                                    } else {
                                        // not the 1st page, simply use SwipeView default behavior
                                        event.accepted = false
                                    }
                                }
                                Keys.onRightPressed: function(event) {
                                    if (gridViewLoader.SwipeView.index === (toplevelRepeater.pageCount - 1) && toplevelRepeater.pageCount > 1) {
                                        // is the last page, go to last page
                                        pages.setCurrentIndex(0)
                                    } else {
                                        // not the last page, simply use SwipeView default behavior
                                        event.accepted = false
                                    }
                                }
                                opacity: folderGridViewPopup.visible ? 0.4 : 1
                                activeGridViewFocusOnTab: gridViewLoader.SwipeView.isCurrentItem
                                itemMove: Transition { NumberAnimation { properties: "x,y"; duration: 250 } }
                                delegate: DropArea {
                                    Keys.forwardTo: [iconItemDelegate]

                                    width: gridViewContainer.cellWidth
                                    height: gridViewContainer.cellHeight
                                    onEntered: {
                                        if (folderGridViewPopup.opened) {
                                            folderGridViewPopup.close()
                                        }
                                    }
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
                                        id: iconItemDelegate
                                        anchors {
                                            fill: parent
                                            margins: 5
                                        }
                                        dndEnabled: !folderGridViewPopup.opened
                                        Drag.mimeData: {
                                            "text/x-dde-launcher-dnd-desktopId": model.desktopId
                                        }
                                        visible: dndItem.currentlyDraggedId !== model.desktopId
                                        iconSource: (iconName && iconName !== "") ? iconName : "application-x-desktop"
                                        icons: folderIcons
                                        onItemClicked: {
                                            launchApp(desktopId)
                                        }
                                        onFolderClicked: {
                                            let idStr = model.desktopId
                                            let idNum = Number(idStr.replace("internal/folders/", ""))
                                            folderGridViewPopup.currentFolderId = idNum
                                            folderGridViewPopup.open()
                                            folderGridViewPopup.folderName = model.display.startsWith("internal/category/") ? getCategoryName(model.display.substring(18)) : model.display
                                            console.log("open folder id:" + idNum)
                                        }
                                        onMenuTriggered: {
                                            showContextMenu(this, model, folderIcons, false, true)
                                        }
                                    }
                                }
                                Connections {
                                    target: pages
                                    function onCurrentIndexChanged() {
                                        gridViewContainer.checkPageSwitchState()
                                    }
                                }
                                Component.onCompleted: {
                                    gridViewContainer.checkPageSwitchState()
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
                    iconSource: iconName
                    width: searchResultGridViewContainer.cellWidth
                    height: searchResultGridViewContainer.cellHeight
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
                columns: 8
                paddingColumns: 0.5
                placeholderIcon: "search_no_result"
                placeholderText: qsTranslate("SearchResultView", "No search results")
                placeholderIconSize: 256
                model: delegateSearchResultModel
                padding: 10
                interactive: true
                vScrollBar: ScrollBar {
                    visible: parent.model.count > 4 * 8
                    active: parent.model.count > 4 * 8
                }
            }
        }


        SearchEdit {
            id: searchEdit

            Layout.alignment: Qt.AlignHCenter
            implicitWidth: (parent.width / 2) > 280 ? 280 : (parent.width / 2)

            property Palette iconPalette: Palette {
                normal {
                    crystal: Qt.rgba(0, 0, 0, 1)
                }
                normalDark {
                    crystal: Qt.rgba(1, 1, 1, 1)
                }
            }
            placeholderTextColor: palette.brightText
            palette.windowText: ColorSelector.iconPalette

            KeyNavigation.up: searchEdit.text === "" ? pages : searchResultGridViewContainer
            KeyNavigation.down: KeyNavigation.up
            Keys.onReturnPressed: {
                if (searchEdit.text === "") {
                    pages.focus = true
                } else {
                    searchResultGridViewContainer.currentItem?.itemClicked()
                }
            }
            onTextChanged: {
                SearchFilterProxyModel.setFilterRegularExpression(text.trim())
                // this can help indirectly reset the currentIndex of the view that the model is attached to
                SearchFilterProxyModel.invalidate()
            }
        }
    }

    FolderGridViewPopup {
        id: folderGridViewPopup
        cs: searchResultGridViewContainer.cellHeight
    }

    Keys.onPressed: {
        if (searchEdit.focus === false && (event.text && !"\t\r\0 ".includes(event.text))) {
            searchEdit.focus = true
            if (searchEdit.text) {
                searchEdit.text += event.text
            } else {
                searchEdit.text = event.text
            }
        } else if (baseLayer.focus === true) {
            // the SearchEdit will catch the key event first, and events that it won't accept will then got here
            switch (event.key) {
            case Qt.Key_Up:
            case Qt.Key_Down:
            case Qt.Key_Enter:
            case Qt.Key_Return:
                pages.focus = true
            }
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
            // close folder popup
            if (folderGridViewPopup.visible) folderGridViewPopup.close()
            // reset(remove) keyboard focus
            baseLayer.focus = true
            // reset page to the first page
            pages.setCurrentIndex(0)
        }
    }
}
