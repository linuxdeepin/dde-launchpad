// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0
import org.kde.kitemmodels 1.0

Control {
    id: root
    visible: true

    property alias model: gridView.model
    property alias delegate: gridView.delegate
    property alias placeholderText: placeholderLabel.text
    property alias interactive: gridView.interactive
    required property int columns
    required property int rows
    property alias cellSize: item.cellSize

    function itemAt(x, y) {
        return gridView.itemAt(x, y)
    }

    function indexAt(x, y) {
        return gridView.indexAt(x, y)
    }

    contentItem: Item {
        id: item
        visible: true

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

                cellHeight: item.cellSize
                cellWidth: item.cellSize

                onCurrentItemChanged: {
                    currentItem.focus = true
                }
            }
        }

        Label {
            id: placeholderLabel
            visible: text !== "" && model.count <= 0
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
