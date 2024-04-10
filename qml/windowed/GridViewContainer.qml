// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

FocusScope {
    id: root
    visible: true
    width: gridView.width
    height: gridView.height

    property alias count: gridView.count
    property alias model: gridView.model
    property alias delegate: gridView.delegate
    property alias interactive: gridView.interactive
    property alias gridViewFocus: gridView.focus
    property bool activeGridViewFocusOnTab: false
    property int columns: 4
    property int rows: Math.min(Math.ceil(count * 1.0 / columns), Helper.windowed.maxViewRows)
    property int paddingColumns: Helper.frequentlyUsed.cellPaddingColumns
    property int paddingRows: Helper.frequentlyUsed.cellPaddingRows
    property real cellHeight: 72
    property real cellWidth: 80

    readonly property alias currentItem: gridView.currentItem
    readonly property alias gridViewWidth: gridView.width
    property alias highlight: gridView.highlight

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

        GridView {
            id: gridView
            width: root.cellWidth * columns + paddingColumns * Math.max(0, columns - 1) + paddingColumns
            height: root.cellHeight * rows + paddingRows * Math.max(0, rows - 1) + paddingRows

            anchors.centerIn: parent

            clip: true
            interactive: false
            highlightFollowsCurrentItem: true
            keyNavigationEnabled: true
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
            cellHeight: root.cellHeight + paddingRows
            cellWidth: root.cellWidth + paddingColumns

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
        }
    }
}
