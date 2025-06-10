// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

FocusScope {
    id: root
    visible: true

    property alias model: gridView.model
    property alias delegate: gridView.delegate
    property alias placeholderIcon: placeholderIcon.name
    property alias placeholderIconSize: placeholderIcon.sourceSize.width
    property alias placeholderText: placeholderLabel.text
    property alias interactive: gridView.interactive
    property alias padding: item.anchors.margins
    property alias gridViewFocus: gridView.focus
    property alias gridViewClip: gridView.clip
    property ScrollBar vScrollBar
    property bool activeGridViewFocusOnTab: false
    property bool alwaysShowHighlighted: false
    property Transition itemMove
    required property int columns
    required property int rows
    property real paddingColumns: 0
    property alias cellHeight: item.cellHeight
    property alias cellWidth: item.cellWidth

    property alias currentIndex: gridView.currentIndex
    readonly property alias currentItem: gridView.currentItem
    readonly property alias gridViewWidth: gridView.width
    readonly property bool isWindowedMode: LauncherController.currentFrame === "WindowedFrame"

    function setPreviousPageSwitch(state) {
        if (state)
            gridView.currentIndex = gridView.count - 1
        else
            gridView.currentIndex = 0
    }

    function positionViewAtBeginning() {
        gridView.positionViewAtBeginning()
    }

    function itemAt(x, y) {
        let point = mapToItem(gridView, x, y)
        return gridView.itemAt(point.x, point.y)
    }

    function indexAt(x, y) {
        let point = mapToItem(gridView, x, y)
        return gridView.indexAt(point.x, point.y)
    }

    Item {
        id: item
        visible: true
        anchors.fill: parent

        property int cellHeight: root.rows == 0 ? (width / (root.columns + root.paddingColumns * 2)) : Math.min(width / (root.columns + root.paddingColumns * 2), height / root.rows)
        property int cellWidth: cellHeight
        Rectangle {
            anchors.centerIn: parent
            width: {
                if (root.objectName === "folderGridViewContainer") {
                    return item.cellWidth * root.columns + root.paddingColumns * Math.max(0, root.columns - 1) + root.paddingColumns
                } else {
                    return item.cellWidth * root.columns
                }
            }
            height: {
                if (root.objectName === "folderGridViewContainer") {
                    return item.cellHeight * root.rows + root.paddingColumns * Math.max(0, root.rows - 1)
                } else {
                    return root.rows == 0 ? parent.height : (item.cellHeight * root.rows)
                }
            }
            color: "transparent"

            GridView {
                id: gridView

                ScrollBar.vertical: root.vScrollBar

                anchors.fill: parent
                clip: true
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                highlightMoveDuration: 100
                activeFocusOnTab: focus ? root.activeGridViewFocusOnTab : false
                focus: count > 0
                onActiveFocusChanged: {
                    if (activeFocus) {
                        let snapMode = gridView.snapMode
                        let preferredHighlightBegin = gridView.preferredHighlightBegin
                        gridView.snapMode = GridView.SnapToRow
                        gridView.preferredHighlightBegin = 0
                        gridView.positionViewAtIndex(gridView.currentIndex, GridView.SnapPosition)
                        gridView.snapMode = snapMode
                        gridView.preferredHighlightBegin = preferredHighlightBegin
                    }
                }
                cellHeight: item.cellHeight
                cellWidth: item.cellWidth

                highlight: Item {
                    FocusBoxBorder {
                        anchors {
                            fill: parent
                            margins: 5
                        }
                        radius: isWindowedMode ? 8 : 18
                        color: parent.palette.highlight
                        visible: gridView.activeFocus
                    }
                    Rectangle {
                        anchors {
                            fill: parent
                            margins: 5    
                        }
                        radius: 18
                        color: Qt.rgba(0, 0, 0, 0.2)
                        visible: alwaysShowHighlighted
                    }
                }

                // working (on drag into folder):
                displaced: root.itemMove
                // not wroking
                move: root.itemMove
                moveDisplaced: root.itemMove

                Keys.onPressed: function (event) {
                    if (event.key === Qt.Key_Right && currentIndex === gridView.count - 1) {
                        gridView.currentIndex = 0;
                        event.accepted = true;
                    } else if (event.key === Qt.Key_Left && currentIndex === 0) {
                        currentIndex = gridView.count - 1;
                        event.accepted = true;
                    }
                }
            }
        }

        ColumnLayout {
            visible: placeholderLabel.text !== "" && model.count <= 0
            anchors.centerIn: parent

            Control {
                id: control
                contentItem: DciIcon {
                    id: placeholderIcon
                    visible: name !== ""
                    sourceSize {
                        width: 128
                        height: width
                    }
                    palette: DTK.makeIconPalette(control.palette)
                    theme: DTK.toColorType(control.palette.window)
                }
            }

            Label {
                id: placeholderLabel
                Layout.alignment: Qt.AlignCenter
            }
        }
    }
}
