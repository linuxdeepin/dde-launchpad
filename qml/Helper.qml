// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
            common: ("transparent")
            crystal: ("transparent")
        }
        hovered {
            common: Qt.rgba(0, 0, 0, 0.1)
            crystal: Qt.rgba(0, 0, 0, 0.1)
        }
        hoveredDark {
            common: Qt.rgba(1, 1, 1, 0.1)
            crystal: Qt.rgba(1, 1, 1, 0.1)
        }
    }

    // Snap a logical pixel value so that value * DPR lands on an integer
    // physical pixel, avoiding sub-pixel texture sampling blur in Scene Graph.
    function pixelAligned(value, dpr) {
        return Math.round(value * dpr) / dpr
    }

    function generateDragMimeData(desktopId, dockOnly = false) {
        // In some cases an app is not allowed to be pinned onto dock via drag-n-drop;
        // We only insert the MIME data for dde-dock in those allowed cases.
        let mime = {}
        if (!dockOnly) {
            mime["text/x-dde-launcher-dnd-desktopId"] = desktopId
        }
        if (!DesktopIntegration.appIsDummyPackage(desktopId)) {
            mime["text/x-dde-dock-dnd-appid"] = desktopId
            mime["text/x-dde-dock-dnd-source"] = "launcher"
        }
        return mime
    }
}
