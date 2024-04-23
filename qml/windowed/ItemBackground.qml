// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

D.BoxPanel {
    id: control
    property Item button
    property D.Palette background: D.Palette {
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

    implicitWidth: DS.Style.toolButton.width
    implicitHeight: DS.Style.toolButton.height
    color1: selectValue(background, DS.Style.checkedButton.background, DS.Style.highlightedButton.background1)
    color2: selectValue(background, DS.Style.checkedButton.background, DS.Style.highlightedButton.background2)
    insideBorderColor: null
    outsideBorderColor: null
    visible: button.checked || button.highlighted || button.visualFocus || control.D.ColorSelector.controlState === D.DTK.PressedState || control.D.ColorSelector.controlState === D.DTK.HoveredState

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
        active: button.visualFocus

        sourceComponent: D.FocusBoxBorder {
            radius: control.radius
            color: button.palette.highlight
        }
    }
}
