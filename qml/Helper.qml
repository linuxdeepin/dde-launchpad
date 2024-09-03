// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.0
import org.deepin.dtk 1.0 as D

QtObject {
    property QtObject windowed :QtObject {
        property int topMargin: 11
        property int splitLineWidth: 1
        property int bottomBarMargins: 10
        property int maxViewRows: 4
        property int doubleRowMaxFontSize: 12
        property int listItemHeight: 36
    }
    property QtObject fullscreen :QtObject {
        property int doubleRowMaxFontSize: 14
    }
    property QtObject frequentlyUsed :QtObject {
        property int leftMargin: 10
        property int rightMargin: 10
        property int cellPaddingColumns: 14
        property int cellPaddingRows: 6
    }

    property D.Palette itemBackground: D.Palette {
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
}
