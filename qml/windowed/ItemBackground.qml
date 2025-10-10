// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

import org.deepin.launchpad 1.0

D.BoxPanel {
    id: control
    property Item button
    property D.Palette background: Helper.itemBackground

    implicitWidth: DS.Style.toolButton.width
    implicitHeight: DS.Style.toolButton.height
    color1: background
    color2: background
    insideBorderColor: null
    outsideBorderColor: null
    visible: {
        // 检查是否有正在进行的拖拽操作
        if (typeof dndItem !== "undefined" && dndItem.currentlyDraggedId !== "") {
            return false
        }
        return button.checked || button.highlighted || button.visualFocus || control.D.ColorSelector.controlState === D.DTK.PressedState || control.D.ColorSelector.controlState === D.DTK.HoveredState
    }

    function selectValue(normal, checked, highlighted) {
        if (button.checked) {
            return checked
        } else if ((typeof button.highlighted == "boolean") && button.highlighted) {
            return highlighted
        }

        return normal
    }

    Loader {
        anchors.fill: parent
        active: button.visualFocus && control.focusPolicy !== Qt.NoFocus

        sourceComponent: D.FocusBoxBorder {
            radius: control.radius
            color: button.palette.highlight
        }
    }
}
