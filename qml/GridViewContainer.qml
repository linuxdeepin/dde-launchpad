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
    property ScrollBar vScrollBar
    property bool activeGridViewFocusOnTab: false
    property Transition itemMove
    required property int columns
    required property int rows
    property int paddingColumns: 0
    property alias cellHeight: item.cellHeight
    property alias cellWidth: item.cellWidth

    readonly property alias currentIndex: gridView.currentIndex
    readonly property alias currentItem: gridView.currentItem
    readonly property alias gridViewWidth: gridView.width

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
                if (root.objectName === "folderGridViewContainer")
                    return model.rowCount() > root.columns - 1 ? item.cellWidth * root.columns : model.rowCount() * item.cellWidth
                else
                    return item.cellWidth * root.columns
            }
            height: rows == 0 ? parent.height : (item.cellHeight * root.rows)
            color: "transparent"

            GridView {
                id: gridView

                ScrollBar.vertical: root.vScrollBar

                anchors.fill: parent
                clip: true
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                highlightMoveDuration: 150
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
                    SystemPalette { id: highlightPalette }
                    FocusBoxBorder {
                        anchors {
                            fill: parent
                            margins: 5
                        }
                        radius: 8
                        color: highlightPalette.highlight
                        visible: gridView.activeFocus
                    }
                }

                // working (on drag into folder):
                displaced: root.itemMove
                // not wroking
                move: root.itemMove
                moveDisplaced: root.itemMove
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
