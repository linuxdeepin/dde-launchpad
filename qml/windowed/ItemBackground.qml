// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

D.BoxPanel {
    visible: D.ColorSelector.controlState === D.DTK.HoveredState
    outsideBorderColor: null
    insideBorderColor: null
    implicitWidth: DS.Style.toolButton.width
    implicitHeight: DS.Style.toolButton.height

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
    color1: background
    color2: background
}
