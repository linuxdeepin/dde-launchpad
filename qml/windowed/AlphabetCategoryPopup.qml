// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import org.deepin.dtk 1.0
import org.deepin.dtk.private 1.0
import org.deepin.dtk.style 1.0 as DStyle

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
            textColor: DStyle.Style.menu.itemText
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
                insideBorderColor: null
                radius: width / 2

                property Palette background: Palette {
                    normal {
                        common: Qt.rgba(0, 0, 0, 0.1)
                        crystal: Qt.rgba(0, 0, 0, 0.1)
                    }
                    normalDark {
                        common: Qt.rgba(1, 1, 1, 0.1)
                        crystal: Qt.rgba(1, 1, 1, 0.1)
                    }
                    hovered {
                        common: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.1)
                        crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.1)
                    }
                }
                color1: background
                color2: background
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
                color: parent.palette.highlight
                visible: alphabetCategoryContainer.activeFocus
            }
        }

       activeFocusOnTab: gridViewFocus
    }

    background: FloatingPanel {
        radius: DStyle.Style.popup.radius
        dropShadowColor: null
    }
}
