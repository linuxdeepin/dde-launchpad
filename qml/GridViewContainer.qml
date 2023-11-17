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
    property bool activeGridViewFocusOnTab: false
    required property int columns
    required property int rows
    property alias cellSize: item.cellSize

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

        property int cellSize: root.rows == 0 ? (width / root.columns) : Math.min(width / root.columns, height / root.rows)

        Rectangle {
            anchors.centerIn: parent
            width: item.cellSize * root.columns
            height: rows == 0 ? parent.height : (item.cellSize * root.rows)
            color: "transparent"

            GridView {
                id: gridView
                anchors.fill: parent
                clip: true
                highlightFollowsCurrentItem: true
                keyNavigationEnabled: true
                activeFocusOnTab: focus ? root.activeGridViewFocusOnTab : false
                focus: count > 0

                cellHeight: item.cellSize
                cellWidth: item.cellSize

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
                displaced: Transition { NumberAnimation { properties: "x,y"; duration: 250 } }
                // not wroking
                move: Transition { NumberAnimation { properties: "x,y"; duration: 250 } }
                moveDisplaced: Transition { NumberAnimation { properties: "x,y"; duration: 250 } }
            }
        }

        ColumnLayout {
            visible: placeholderLabel.text !== "" && model.count <= 0
            anchors.centerIn: parent

            DciIcon {
                id: placeholderIcon
                visible: name !== ""
                sourceSize {
                    width: 128
                    height: width
                }
            }

            Label {
                id: placeholderLabel
                Layout.alignment: Qt.AlignCenter
            }
        }
    }
}
