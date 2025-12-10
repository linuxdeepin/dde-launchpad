// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DStyle

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import org.deepin.launchpad.windowed 1.0 as Windowed

Popup {
    id: root

    property alias currentFolderId: folderLoader.currentFolderId
    property alias innerItem: folderLoader.item
    property alias folderName: folderLoader.folderName
    property var currentDragItem: null



    function onDragEnter(item) {
        currentDragItem = item
    }

    function onDragExit(item) {
        if (currentDragItem === item) {
            currentDragItem = null
            Qt.callLater(function() {
                if (currentDragItem === null) {
                    root.close()
                }
            })
        }
    }
    property var folderNameFont: DTK.fontManager.t2
    required property point centerPosition
    readonly property bool isWindowedMode: LauncherController.currentFrame === "WindowedFrame"

    modal: true
    focus: true
    // visible: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property int cs: 110 // * 5 / 4
    // anchors.centerIn: parent // seems dtkdeclarative's Popup doesn't have anchors.centerIn

    width: cs * 4 + 20 /* padding */

    // TODO: 经验证发现：Poppu窗口高度为奇数时，会多显示一个像素的外边框；为偶数时不会显示
    // 因此，这里需要保证高度是偶数来确保Popup窗口没有外边框
    height: ((cs * 3) % 2 === 0 ? (cs * 3) : (cs * 3 + 1)) + 130 /* title height*/
    x: centerPosition.x - (width / 2)
    y: centerPosition.y - (height / 2)

    onClosed: {
        // reset folder view
        folderLoader.currentFolderId = -1
        currentDragItem = null
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

            contentItem: Item {
                anchors.fill: parent

                DropArea {
                    anchors.fill: parent
                    keys: ["text/x-dde-launcher-dnd-desktopId"]
                    onEntered: root.onDragEnter(this)
                    onExited: root.onDragExit(this)
                }

                ColumnLayout {
                    id: contentRoot
                    anchors.fill: parent
                    spacing: isWindowedMode ? 0 : 5

                    property bool nameEditing: false
                    property alias folderName: folderNameEdit.text
                    property int titleMargin: isWindowedMode ? 20 : 30

                    signal closeFolder()

                property Palette titleTextColor: Palette {
                    normal {
                        common: Qt.rgba(0, 0, 0, 1)
                        crystal: Qt.rgba(0, 0, 0, 1)
                    }
                    normalDark {
                        common: Qt.rgba(1, 1, 1, 1)
                        crystal: Qt.rgba(1, 1, 1, 1)
                    }
                }

                TextInput {
                    id: folderNameEdit
                    Layout.fillWidth: true
                    Layout.leftMargin: contentRoot.titleMargin - root.padding
                    Layout.rightMargin: contentRoot.titleMargin - root.padding
                    visible: contentRoot.nameEditing
                    clip: true
                    font: folderNameFont
                    horizontalAlignment: Text.AlignHCenter
                    text: folderLoader.folderName
                    color: palette.windowText
                    selectByMouse: true
                    maximumLength: 255
                    onEditingFinished: {
                        contentRoot.nameEditing = false
                        if (text === "") {
                            text = folderLoader.folderName
                            return
                        }
                        ItemArrangementProxyModel.updateFolderName(folderLoader.currentFolderId, text);
                        folderNameText.text = text
                    }

                    // TODO: selectionColor will work after dtkdeclarative fix background blur isssues
                    selectionColor: palette.highlight
                }
                Text {
                    id: folderNameText
                    Layout.fillWidth: true
                    Layout.leftMargin: contentRoot.titleMargin - root.padding
                    Layout.rightMargin: contentRoot.titleMargin - root.padding
                    clip: true
                    font: folderNameFont
                    horizontalAlignment: Text.AlignHCenter
                    text: folderLoader.folderName
                    color: contentRoot.ColorSelector.titleTextColor
                    visible: !contentRoot.nameEditing
                    elide: Text.ElideRight
                    ToolTip.visible: folderNameTextMouseArea.containsMouse ? implicitWidth > width : false
                    ToolTip.delay: 500
                    ToolTip.timeout: 5000
                    ToolTip.text: text

                    MouseArea {
                        id: folderNameTextMouseArea
                        hoverEnabled: true
                        anchors.fill: parent

                        onClicked: {
                            contentRoot.nameEditing = true
                            folderNameEdit.forceActiveFocus()
                            folderNameEdit.selectAll()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"

                    DropArea {
                        id: folderPageDropArea
                        property int pageIntent: 0
                        readonly property real paddingColumns: 0.3
                        readonly property int horizontalPadding: contentRoot.width * paddingColumns
                        anchors.fill: parent
                        property bool createdEmptyPage: false

                        onEntered: root.onDragEnter(this)

                        onExited: {
                            root.onDragExit(this)
                            pageIntent = 0
                            createdEmptyPage = false
                        }

                        function checkDragMove() {
                            if (drag.x < horizontalPadding) {
                                pageIntent = -1
                            } else if (drag.x > (width - horizontalPadding)) {
                                let isLastPage = folderPagesView.currentIndex === folderPagesView.count - 1
                                if (isLastPage && folderPageDropArea.createdEmptyPage) {
                                    return
                                }
                                pageIntent = 1
                            } else {
                                pageIntent = 0
                            }
                        }

                        keys: ["text/x-dde-launcher-dnd-desktopId"]
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
                            dropOnPage(dragId, "internal/folders/" + folderLoader.currentFolderId, folderPagesView.currentIndex)
                            pageIntent = 0
                            createdEmptyPage = false
                        }
                        onPageIntentChanged: {
                            if (pageIntent !== 0) {
                                folderDndMovePageTimer.restart()
                            } else {
                                folderDndMovePageTimer.stop()
                            }
                        }

                        Timer {
                            id: folderDndMovePageTimer
                            interval: 1000
                            onTriggered: {
                                if (parent.pageIntent > 0) {
                                    let isLastPage = (folderPagesView.currentIndex === folderPagesView.count - 1)
                                    if (isLastPage && !folderPageDropArea.createdEmptyPage) {
                                        let newPageIndex = ItemArrangementProxyModel.creatEmptyPage(folderLoader.currentFolderId)
                                        folderPageDropArea.createdEmptyPage = true
                                        folderPagesView.setCurrentIndex(newPageIndex)
                                        parent.pageIntent = 0
                                        return
                                    } else {
                                        incrementPageIndex(folderPagesView)
                                    }
                                } else if (parent.pageIntent < 0) {
                                    decrementPageIndex(folderPagesView)
                                }
                                parent.pageIntent = 0
                                if (folderPagesView.currentIndex !== 0) {
                                    parent.checkDragMove()
                                }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        scrollGestureEnabled: false

                        // TODO: this might not be the correct way to handle wheel
                        onWheel: function (wheel) {
                            let xDelta = wheel.angleDelta.x / 8
                            let yDelta = wheel.angleDelta.y / 8
                            let toPage = 0; // -1 prev, +1 next, 0 don't change
                            if (yDelta !== 0) {
                                toPage = (yDelta > 0) ? -1 : 1
                            } else if (xDelta !== 0) {
                                toPage = (xDelta > 0) ? 1 : -1
                            }
                            if (toPage < 0) {
                                decrementPageIndex(folderPagesView)
                            } else if (toPage > 0) {
                                incrementPageIndex(folderPagesView)
                            }
                        }
                    }

                    SwipeView {
                        id: folderPagesView
                        clip: gridViews.count > 1

                        anchors.fill: parent

                        currentIndex: folderPageIndicator.currentIndex
                        activeFocusOnTab: false
                        
                        // 处理页面切换时的焦点传递
                        onCurrentIndexChanged: {
                            if (currentItem)
                                currentItem.resetCurrentIndex()
                        }

                        Connections {
                            target: ItemArrangementProxyModel
                            function onFolderPageCountChanged(folderId) {
                                if (folderId === folderLoader.currentFolderId) {
                                    gridViews.model = ItemArrangementProxyModel.pageCount(folderId)
                                }
                            }
                        }
                        Repeater {
                            id: gridViews
                            model: ItemArrangementProxyModel.pageCount(folderLoader.currentFolderId) // FIXME: should be a property?

                            Loader {
                                active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem
                                id: folderGridViewLoader
                                objectName: "Folder GridView Loader"

                                function resetCurrentIndex() {
                                    if (item && item.resetGridFocus)
                                        item.resetGridFocus()
                                }

                                sourceComponent: Rectangle {
                                    anchors.fill: parent
                                    color: "transparent"
                                    
                                    function resetGridFocus() {
                                        if (gridViewContainerLoader && gridViewContainerLoader.item) {
                                            gridViewContainerLoader.item.currentIndex = 0
                                            gridViewContainerLoader.item.forceActiveFocus()
                                        } 
                                    }

                                    MultipageSortFilterProxyModel {
                                        id: folderProxyModel
                                        filterOnlyMode: true
                                        sourceModel: ItemArrangementProxyModel
                                        pageId: modelData
                                        folderId: folderLoader.currentFolderId
                                    }
                                    
                                    SortProxyModel {
                                        id: sortProxyModel
                                        sourceModel: folderProxyModel
                                        sortRole: ItemArrangementProxyModel.IndexInPageRole
                                        Component.onCompleted: {
                                            sortProxyModel.sort(0)
                                        }
                                    }

                                    //gridViewContainer
                                    Loader {
                                        id: gridViewContainerLoader
                                        anchors.fill: parent
                                        sourceComponent: isWindowedMode ? listViewGridViewContainer : fullScreenGridViewContainer
                                        
                                        // Define common itemMove transition to avoid duplication
                                        property Transition itemMove: Transition {
                                            NumberAnimation {
                                                properties: "x,y"
                                                duration: 200
                                                easing.type: Easing.OutQuad
                                            }
                                        }
                                    }

                                    Component {
                                        id: fullScreenGridViewContainer
                                        GridViewContainer {
                                            id: folderGridViewContainer
                                            objectName: "folderGridViewContainer"
                                            anchors.fill: parent
                                            rows: 3
                                            columns: 4
                                            model: sortProxyModel
                                            padding: 10
                                            interactive: false
                                            focus: true
                                            gridViewClip: false // TODO it maybe a bug for dtk, https://github.com/linuxdeepin/developer-center/issues/8468
                                            activeGridViewFocusOnTab: folderGridViewLoader.SwipeView.isCurrentItem
                                            itemMove: parent.itemMove
                                            onActiveFocusChanged: {
                                                if (activeFocus) {
                                                    currentIndex = 0
                                                }
                                            }
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
                                            model: sortProxyModel
                                            paddingRows: 6
                                            cellHeight: 86
                                            paddingColumns: 2
                                            interactive: false
                                            focus: true
                                            gridViewClip: false
                                            activeGridViewFocusOnTab: folderGridViewLoader.SwipeView.isCurrentItem
                                            itemMove: parent.itemMove

                                            onActiveFocusChanged: {
                                                if (activeFocus) {
                                                    currentIndex = 0
                                                }
                                            }
                                            delegate: DelegateDropArea {
                                                width: folderGridViewContainer.cellWidth
                                                height: folderGridViewContainer.cellHeight
                                            }
                                        }
                                    }

                                    component DelegateDropArea: DropArea {
                                        onEntered: function(drag) {
                                            root.onDragEnter(this)
                                            folderDragApplyTimer.dragId = drag.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                                            folderDragApplyTimer.restart()
                                        }
                                        onPositionChanged: function(drag) {
                                            let dragId = drag.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                                            if (dragId === model.desktopId) {
                                                return
                                            }
                                            folderDragApplyTimer.dragId = dragId
                                            folderDragApplyTimer.currentDropX = drag.x
                                            if (!folderDragApplyTimer.running) {
                                                folderDragApplyTimer.restart()
                                            }
                                        }
                                        onExited: {
                                            root.onDragExit(this)
                                            folderDragApplyTimer.stop()
                                            folderDragApplyTimer.dragId = ""
                                        }
                                        Component.onDestruction: {
                                            root.onDragExit(this)
                                        }
                                        onDropped: function(drop) {
                                            let dragId = drop.getDataAsString("text/x-dde-launcher-dnd-desktopId")
                                            if (dragId === "") {
                                                return
                                            }
                                            if (folderDragApplyTimer.running) {
                                                folderDragApplyTimer.stop()
                                                root.itemDropped(dragId) // This seems to be a new call for general item drop handling
                                            }

                                            folderDragApplyTimer.stop()
                                            folderDragApplyTimer.dragId = ""
                                            // The following logic is for dropping onto an item specifically
                                            if (dragId === model.desktopId) {
                                                return
                                            }

                                            let op = 1 // DndPrepend = -1, DndJoin = 0, DndAppend = 1
                                            let sideOpPadding = width / 2
                                            if (drop.x < sideOpPadding) {
                                                op = -1
                                            }

                                            dropOnItem(dragId, model.desktopId, op)
                                            sortProxyModel.sort(0)
                                        }
                                        
                                        Timer {
                                            id: folderDragApplyTimer
                                            interval: 400
                                            property string dragId: ""
                                            property real currentDropX: 0
                                            onTriggered: function() {
                                                if (dragId === "") return
                                                let op = 0
                                                let sideOpPadding = parent.width / 4
                                                if (currentDropX < sideOpPadding) {
                                                    op = -1
                                                } else if (currentDropX > (parent.width - sideOpPadding)) {
                                                    op = 1
                                                }
                                                // 只有在需要插入操作时才执行预览移动
                                                if (op !== 0) {
                                                    dropOnItem(dragId, model.desktopId, op)
                                                    sortProxyModel.sort(0)
                                                }
                                            }
                                        }
                                        Keys.forwardTo: [innerItem]

                                        IconItemDelegate {
                                            id: innerItem
                                            anchors.fill: parent
                                            dndEnabled: true
                                            displayFont: isWindowedMode ? DTK.fontManager.t9 : DTK.fontManager.t6
                                            Drag.mimeData: Helper.generateDragMimeData(model.desktopId)
                                            visible: dndItem.currentlyDraggedId !== model.desktopId
                                            iconSource: iconName

                                            padding: isWindowedMode ? 0 : 5
                                            onItemClicked: {
                                                launchApp(desktopId)
                                            }
                                            onMenuTriggered: {
                                                showContextMenu(this, model)
                                                baseLayer.focus = true
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        anchors.fill: parent
                        MouseArea {
                            anchors.fill: parent
                            enabled: contentRoot.nameEditing
                            onClicked: {
                                folderNameEdit.editingFinished()
                            }
                        }
                    }
                }

                PageIndicator {
                    Layout.alignment: Qt.AlignHCenter
                    visible: folderPagesView.count > 1 ? true : false
                    id: folderPageIndicator
                    implicitHeight: isWindowedMode ? 13 : folderPageIndicator.implicitWidth
                    count: folderPagesView.count
                    currentIndex: folderPagesView.currentIndex
                    interactive: true
                    spacing: isWindowedMode ? 5 : 10

                    delegate: Rectangle {
                        implicitWidth: isWindowedMode ? 5 : 10
                        implicitHeight: isWindowedMode ? 5 : 10

                        radius: width / 2
                        color: Qt.rgba(255, 255, 255, index === folderPageIndicator.currentIndex ? 0.9 : pressed ? 0.5 : 0.2)
                        Behavior on opacity { OpacityAnimator { duration: 100 } }
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
    }
    }
    background: Item {
        FloatingPanel {
            anchors.fill: parent
            radius: isWindowedMode ? 12 : 36
            blurMultiplier: 5.0
            backgroundColor: Palette {
                normal: Qt.rgba(1.0, 1.0, 1.0, 0.2)
                normalDark: isWindowedMode ? Qt.rgba(20/255, 20/255, 20/255, 0.4) : Qt.rgba(1.0, 1.0, 1.0, 0.2)
            }
            dropShadowColor: null
            outsideBorderColor: isWindowedMode ? windowedOutBorderPalette : null
            insideBorderColor: isWindowedMode ? DStyle.Style.floatingPanel.insideBorder : null
        }

        DropArea {
            anchors.fill: parent
            keys: ["text/x-dde-launcher-dnd-desktopId"]
            onEntered: root.onDragEnter(this)
            onExited: root.onDragExit(this)
        }
    }

    Palette {
        id: windowedOutBorderPalette
        normal: Qt.rgba(0, 0, 0, 0.06)
        normalDark: Qt.rgba(0, 0, 0, 0.4)
    }
}
