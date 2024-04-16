// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import org.deepin.dtk 1.0
import org.deepin.dtk.private 1.0

import org.deepin.launchpad.windowed 1.0 as Windowed

Popup {
    id: root
    modal: true
    dim: true
    padding: 0
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal categoryClicked(string character)

    property alias existingSections: alphabetCategoryDelegateModel.model
    property int columns: 5

    readonly property int cellWidth: 24
    readonly property int cellHeight: 24
    readonly property int paddingColumns: 6
    readonly property int paddingRows: 16

    width: alphabetCategoryContainer.width + 20
    height: alphabetCategoryContainer.height + 20

    DelegateModel {
        id: alphabetCategoryDelegateModel

        delegate: ToolButton {
            width: alphabetCategoryContainer.cellWidth + paddingColumns
            height: alphabetCategoryContainer.cellHeight + paddingRows
            text: modelData
            focusPolicy: Qt.NoFocus
            onClicked: {
                categoryClicked(modelData)
            }

            background: ButtonPanel {
                button: parent
                anchors.centerIn: parent
                width: root.cellWidth
                height: root.cellHeight
                outsideBorderColor: null
                radius: width / 2
            }
        }
    }

    Windowed.GridViewContainer {
        id: alphabetCategoryContainer

        anchors.centerIn: parent

        model: alphabetCategoryDelegateModel
        columns: root.columns
        rows: model.count / columns + (model.count % columns > 0 ? 1 : 0)
        cellWidth: root.cellWidth
        cellHeight: root.cellHeight
        paddingColumns: root.paddingColumns
        paddingRows: root.paddingRows

        highlight: Item {
            FocusBoxBorder {
                anchors.centerIn: parent
                width: root.cellWidth
                height: root.cellHeight
                radius: root.cellWidth / 2
                color: palette.highlight
                visible: alphabetCategoryContainer.activeFocus
            }
        }

       activeFocusOnTab: gridViewFocus
    }
}
