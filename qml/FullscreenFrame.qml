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
import 'windowed' as WindowedLaunchpad

InputEventItem {
    anchors.fill: parent
    objectName: "FullscreenFrame-InputEventItem"

    property Palette appTextColor: Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 1)
            crystal: Qt.rgba(0, 0, 0, 1)
        }
        normalDark {
            common: Qt.rgba(1, 1, 1, 0.7)
            crystal: Qt.rgba(1, 1, 1, 0.7)
        }
    }

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

        function tryToRemoveEmptyPage() {
            ItemArrangementProxyModel.removeEmptyPage()
        }

        Timer {
            id: flipPageDelay
            interval: 400
            repeat: false
        }

        background: Image {
            source: DesktopIntegration.isTreeLand() ? undefined : DesktopIntegration.backgroundUrl
            sourceSize: Qt.size(width / 3, height / 3)

            Rectangle {
                anchors.fill: parent
                color: folderGridViewPopup.visible ? Qt.rgba(0, 0, 0, 0.6)
                                                   : DesktopIntegration.isTreeLand()
                                                       ? "transparent"
                                                       : Qt.rgba(0, 0, 0, 0.5)

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
                            decrementPageIndex(listviewPage)
                        } else if (toPage > 0) {
                            flipPageDelay.start()
                            if (!searchEdit.focus) { // reset keyboard focus when using mouse to flip page, but keep searchEdit focus
                                baseLayer.focus = true
                            }
                            incrementPageIndex(listviewPage)
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

                contentItem: Rectangle {
                    id: fullscreenHeader
                    implicitHeight: exitFullscreenBtn.height
                    opacity: folderGridViewPopup.visible ? 0.4 : 1
                    color: "transparent"

                    ToolButton {
                        id: exitFullscreenBtn
                        Accessible.name: "Exit fullscreen"
                        anchors.right: fullscreenHeader.right
                        ColorSelector.family: Palette.CrystalColor
                        icon.name: "launcher_exit_fullscreen"
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: qsTr("Window Mode")
                        background: WindowedLaunchpad.ItemBackground {
                            button: exitFullscreenBtn
                        }
                        onClicked: {
                            searchEdit.text = ""
                            LauncherController.currentFrame = "WindowedFrame"
                        }
                    }

                    PageIndicator {
                        id: indicator

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        visible: listviewPage.visible
                        count: searchResultGridViewContainer.visible ? 1 : listviewPage.count
                        currentIndex: searchResultGridViewContainer.visible ? 1 : listviewPage.currentIndex
                        interactive: true
                        spacing: 10
                        delegate: Rectangle {
                            width: 8
                            height: 8

                            radius: width / 2
                            color: Qt.rgba(255, 255, 255, index === indicator.currentIndex ? 0.9 : pressed ? 0.5 : 0.2)
                            OutsideBoxBorder {
                                anchors.fill: parent
                                radius: parent.radius
                                width: 1
                                color: Qt.rgba(0, 0, 0, 0.1)
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

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
                            let isLastPage = listviewPage.currentIndex === listviewPage.count - 1
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
                                   dropOnPage(dragId, "internal/folders/0", listviewPage.currentIndex)
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
                                let isLastPage = listviewPage.currentIndex === listviewPage.count - 1
                                if (isLastPage && !dropArea.createdEmptyPage) {
                                    let newPageIndex = ItemArrangementProxyModel.creatEmptyPage()
                                    dropArea.createdEmptyPage = true
                                    listviewPage.setCurrentIndex(newPageIndex)
                                    parent.pageIntent = 0
                                    return
                                } else {
                                    incrementPageIndex(listviewPage)
                                }
                            } else if (parent.pageIntent < 0) {
                                decrementPageIndex(listviewPage)
                            }

                            parent.pageIntent = 0
                            if (listviewPage.currentIndex !== 0) {
                                parent.checkDragMove()
                            }
                        }
                    }

                    Connections {
                        target: dndItem
                        function onDragEnded() {
                            if (dropArea.createdEmptyPage) {
                                baseLayer.tryToRemoveEmptyPage()
                                dropArea.createdEmptyPage = false
                            }
                        }
                    }
                }

                ItemsPageModel {
                    id: itemPageModel
                    sourceModel: ItemArrangementProxyModel
                }

                ListView {
                    id: listviewPage

                    anchors.fill: parent
                    snapMode: ListView.SnapOneItem
                    orientation: ListView.Horizontal
                    highlightRangeMode: ListView.StrictlyEnforceRange
                    highlightFollowsCurrentItem: true
                    highlightMoveDuration: 200
                    highlightMoveVelocity: -1

                    activeFocusOnTab: true
                    focus: true
                    visible: searchEdit.text === ""

                    currentIndex: indicator.currentIndex
                    function setCurrentIndex(index) {
                        listviewPage.currentIndex = index
                        listviewPage.currentIndex = Qt.binding(function() { return indicator.currentIndex })
                    }

                    property int previousIndex: -1
                    model: itemPageModel

                    delegate: FocusScope {
                        id: listItem
                        width: listviewPage.width
                        height: listviewPage.height

                        property int viewIndex: index

                        SortProxyModel {
                            id: proxyModel
                            sourceModel: MultipageSortFilterProxyModel {
                                filterOnlyMode: true
                                sourceModel: ItemArrangementProxyModel
                                pageId: viewIndex
                                folderId: 0
                            }
                            sortRole: ItemArrangementProxyModel.IndexInPageRole
                            Component.onCompleted: {
                                proxyModel.sort(0)
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton | Qt.LeftButton
                            onClicked: {
                                // FIXME: prevent the bug:https://bugreports.qt.io/browse/QTBUG-125139;
                                if (mouse.button === Qt.RightButton) {
                                    mouse.accepted = false;
                                } else if (!DebugHelper.avoidHideWindow) {
                                    LauncherController.visible = false
                                }
                            }
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
                                if (listItem.viewIndex !== listviewPage.currentIndex)
                                    return
                                if (listviewPage.previousIndex === -1) {
                                    listviewPage.previousIndex = listviewPage.currentIndex
                                    return
                                }
                                if (listviewPage.currentIndex + 1 === listviewPage.previousIndex || (listviewPage.previousIndex === 0 && listviewPage.currentIndex === listviewPage.count - 1)) {
                                    gridViewContainer.setPreviousPageSwitch(true)
                                } else {
                                    gridViewContainer.setPreviousPageSwitch(false)
                                }
                                listviewPage.previousIndex = listviewPage.currentIndex
                            }

                            Keys.onLeftPressed: function(event) {
                                if (listItem.viewIndex === 0 && itemPageModel.rowCount() > 1) {
                                    // is the 1st page, go to last page
                                    listviewPage.setCurrentIndex(itemPageModel.rowCount() - 1)
                                } else {
                                    // not the 1st page, simply use SwipeView default behavior
                                    event.accepted = false
                                }
                            }
                            Keys.onRightPressed: function(event) {
                                if (listItem.viewIndex === (itemPageModel.rowCount() - 1) && itemPageModel.rowCount() > 1) {
                                    // is the last page, go to last page
                                    listviewPage.setCurrentIndex(0)
                                } else {
                                    // not the last page, simply use SwipeView default behavior
                                    event.accepted = false
                                }
                            }
                            opacity: folderGridViewPopup.visible ? 0.2 : 1
                            Behavior on opacity {
                                NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
                            }
                            activeGridViewFocusOnTab: listviewPage.ListView.isCurrentItem
                            itemMove: Transition {
                                NumberAnimation {
                                    properties: "x,y"
                                    duration: 200
                                    easing.type: Easing.OutQuad
                                }
                            }
                            delegate: DropArea {
                                Keys.forwardTo: [iconItemDelegate]

                                visible: !folderGridViewPopup.visible || folderGridViewPopup.currentFolderId !== Number(model.desktopId.replace("internal/folders/", ""))
                                width: gridViewContainer.cellWidth
                                height: gridViewContainer.cellHeight
                                onEntered: function (drag) {
                                    if (folderGridViewPopup.opened) {
                                        folderGridViewPopup.close()
                                    }
                                    dndDropEnterTimer.dragId = drag.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                                    dndDropEnterTimer.restart()
                                }
                                onExited: {
                                    dndDropEnterTimer.stop()
                                    dndDropEnterTimer.dragId = ""
                                }
                                onDropped: function (drop) {
                                    dndDropEnterTimer.stop()
                                    dndDropEnterTimer.dragId = ""
                                    let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                                    let op = 0
                                    let sideOpPadding = width / 4
                                    if (drop.x < sideOpPadding) {
                                        op = -1
                                    } else if (drop.x > (width - sideOpPadding)) {
                                        op = 1
                                    }
                                    dropOnItem(dragId, model.desktopId, op)
                                    proxyModel.sort(0)
                                }

                                Timer {
                                    id: dndDropEnterTimer
                                    interval: 500
                                    property string dragId: ""
                                    onTriggered: function() {
                                        if (dragId === "") return
                                        let op = 0
                                        let sideOpPadding = width / 4
                                        if (drag.x < sideOpPadding) {
                                            op = -1
                                        } else if (drag.x > (width - sideOpPadding)) {
                                            op = 1
                                        }
                                        if (op === 0) {
                                            dndDropEnterTimer.restart()
                                            return
                                        }
                                        dropOnItem(dragId, model.desktopId, op)
                                        proxyModel.sort(0)
                                    }
                                }

                                IconItemDelegate {
                                    id: iconItemDelegate
                                    anchors {
                                        fill: parent
                                        margins: 5
                                    }
                                    dndEnabled: !folderGridViewPopup.opened
                                    Drag.mimeData: Helper.generateDragMimeData(model.desktopId)
                                    visible: dndItem.currentlyDraggedId !== model.desktopId
                                    iconSource: (iconName && iconName !== "") ? iconName : "application-x-desktop"
                                    icons: folderIcons
                                    onItemClicked: {
                                        launchApp(desktopId)
                                    }
                                    onFolderClicked: {
                                        let idStr = model.desktopId
                                        let idNum = Number(idStr.replace("internal/folders/", ""))
                                        let itemPos = mapToItem(baseLayer, x, y)
                                        folderGridViewPopup.currentFolderId = idNum
                                        folderGridViewPopup.startPointX = itemPos.x + width / 2
                                        folderGridViewPopup.startPointY = itemPos.y + height / 2
                                        folderGridViewPopup.open()
                                        folderGridViewPopup.folderName = model.display.startsWith("internal/category/") ? getCategoryName(model.display.substring(18)) : model.display
                                        console.log("open folder id:" + idNum)
                                    }
                                    onMenuTriggered: {
                                        if (folderIcons) return;
                                        showContextMenu(this, model)
                                        baseLayer.focus = true
                                    }
                                }
                            }
                            Connections {
                                target: listviewPage
                                function onCurrentIndexChanged() {
                                    gridViewContainer.checkPageSwitchState()
                                }
                            }
                            Connections {
                                target: dropArea
                                function onDropped() {
                                    gridViewContainer.checkPageSwitchState()
                                }
                            }
                            Component.onCompleted: {
                                gridViewContainer.checkPageSwitchState()
                            }
                            Component.onDestruction: {
                            }                            
                        }
                    }

                    Component.onCompleted: {
                        listviewPage.setCurrentIndex(0)
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
                            showContextMenu(this, model)
                        }
                    }
                }

                GridViewContainer {
                    id: searchResultGridViewContainer

                    anchors.fill: parent
                    visible: searchEdit.text !== ""
                    activeFocusOnTab: visible && gridViewFocus
                    focus: true
                    alwaysShowHighlighted: true

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
                opacity: folderGridViewPopup.visible ? 0.4 : 1

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

                KeyNavigation.up: searchEdit.text === "" ? listviewPage : searchResultGridViewContainer
                KeyNavigation.down: KeyNavigation.up
                Keys.onReturnPressed: {
                    if (searchEdit.text === "") {
                        listviewPage.focus = true
                    } else {
                        searchResultGridViewContainer.currentItem?.itemClicked()
                    }
                }
                onTextChanged: {
                    searchEdit.focus = true
                    SearchFilterProxyModel.setFilterRegularExpression(text.trim())
                    // reset highlighted item
                    if (searchResultGridViewContainer.visible) {
                        if (delegateSearchResultModel.count > 0) {
                            searchResultGridViewContainer.currentIndex = 0
                        }
                    }
                }
            }
        }

        FolderGridViewPopup {
            id: folderGridViewPopup
            cs: searchResultGridViewContainer.cellHeight
            centerPosition: Qt.point(curPointX, curPointY)

            property int startPointX: 0
            property int startPointY: 0
            readonly property point endPoint: Qt.point((parent.width - parent.rightPadding + parent.leftPadding) / 2, (parent.height - parent.bottomPadding + parent.topPadding) / 2)
            property int curPointX: 0
            property int curPointY: 0

            enter: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        duration: 200
                        properties: "scale"
                        easing.type: Easing.OutQuad
                        from: 0.2
                        to: 1
                    }
                    NumberAnimation {
                        duration: 200
                        properties: "curPointX"
                        easing.type: Easing.OutQuad
                        from: folderGridViewPopup.startPointX
                        to: folderGridViewPopup.endPoint.x
                    }
                    NumberAnimation {
                        duration: 200
                        properties: "curPointY"
                        easing.type: Easing.OutQuad
                        from: folderGridViewPopup.startPointY
                        to: folderGridViewPopup.endPoint.y
                    }
                }
            }

            exit: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        duration: 200
                        properties: "scale"
                        easing.type: Easing.InQuad
                        from: 1
                        to: 0.2
                    }
                    NumberAnimation {
                        duration: 200
                        properties: "curPointX"
                        easing.type: Easing.InQuad
                        to: folderGridViewPopup.startPointX
                        from: folderGridViewPopup.endPoint.x
                    }
                    NumberAnimation {
                        duration: 200
                        properties: "curPointY"
                        easing.type: Easing.InQuad
                        to: folderGridViewPopup.startPointY
                        from: folderGridViewPopup.endPoint.y
                    }
                }
            }
        }

        Keys.forwardTo: [searchEdit]
        Keys.onPressed: {
            if (baseLayer.focus === true) {
                // the SearchEdit will catch the key event first, and events that it won't accept will then got here
                switch (event.key) {
                case Qt.Key_Up:
                case Qt.Key_Down:
                case Qt.Key_Left:
                case Qt.Key_Right:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                    listviewPage.focus = true
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
                listviewPage.setCurrentIndex(0)
            }
        }
    }
    onInputReceived: function(text){
        if (searchEdit.text !== "" || searchEdit.focus !== true) {
            searchEdit.text = text
            searchEdit.focus = true
        }
    }
}
