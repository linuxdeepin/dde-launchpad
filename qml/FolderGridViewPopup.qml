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
    property alias folderName: folderLoader.folderName
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
                id: contentRoot
                spacing: isWindowedMode ? 0 : 5
                anchors.fill: parent
                property bool nameEditing: false
                property int titleMargin: isWindowedMode ? 20 : 30

                Item {
                    visible: !isWindowedMode
                    Layout.preferredHeight: 15
                }

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

                        Repeater {
                            id: gridViews
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
                                            objectName: "folderGridViewContainer"
                                            anchors.fill: parent
                                            rows: 3
                                            columns: 4
                                            model: folderProxyModel
                                            padding: 10
                                            interactive: false
                                            focus: true
                                            gridViewClip: false // TODO it maybe a bug for dtk, https://github.com/linuxdeepin/developer-center/issues/8468
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
                                            cellHeight: 86
                                            paddingColumns: 2
                                            interactive: false
                                            focus: true
                                            gridViewClip: false
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
    background: FloatingPanel {
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

    Palette {
        id: windowedOutBorderPalette
        normal: Qt.rgba(0, 0, 0, 0.06)
        normalDark: Qt.rgba(0, 0, 0, 0.4)
    }
}
