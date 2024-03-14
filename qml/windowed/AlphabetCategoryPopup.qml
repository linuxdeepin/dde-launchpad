// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Popup {
    id: gridPopup
    width: 180
    modal: true
    dim: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal categoryClicked(string character)

    property var existingSections: []

    DelegateModel {
        id: alphabetCategoryDelegateModel
        model: [
            '&', '#', 'A', 'B',
            'C', 'D', 'E', 'F',
            'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N',
            'O', 'P', 'Q', 'R',
            'S', 'T', 'U', 'V',
            'W', 'X', 'Y', 'Z',
        ]
        delegate: ToolButton {
            width: alphabetCategoryContainer.cellWidth
            height: alphabetCategoryContainer.cellHeight
            text: modelData
            focusPolicy: Qt.NoFocus
            enabled: gridPopup.existingSections.includes(modelData)
            onClicked: {
                categoryClicked(modelData)
            }
        }
    }

    GridViewContainer {
        id: alphabetCategoryContainer

        width: parent.width
        height: parent.height

        anchors.fill: parent
        activeGridViewFocusOnTab: true
        model: alphabetCategoryDelegateModel
        rows: 6
        columns: 5
        padding: 2
    }
}
