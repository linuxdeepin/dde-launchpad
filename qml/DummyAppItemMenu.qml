// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Loader {
    id: root

    property string desktopId
    readonly property bool isFullscreen: LauncherController.currentFrame === "FullscreenFrame"
    readonly property bool isHorizontalDock: DesktopIntegration.dockPosition === Qt.UpArrow || DesktopIntegration.dockPosition === Qt.DownArrow
    readonly property int dockSpacing: (isHorizontalDock ? DesktopIntegration.dockGeometry.height : DesktopIntegration.dockGeometry.width) / Screen.devicePixelRatio

    signal closed()

    Component {
        id: contextMenuComp

        Menu {
            id: contextMenu
            margins: isFullscreen ? dockSpacing : 0
            modal: true

            MenuItem {
                text: qsTr("Install")
                onTriggered: {
                    launchApp(root.desktopId)
                }
            }

            MenuItem {
                text: qsTr("Remove")
                onTriggered: {
                    DesktopIntegration.uninstallApp(root.desktopId)
                }
            }

            onClosed: {
                root.closed()
                root.destroy()
            }
        }
    }

    Connections {
        target: LauncherController
        function onVisibleChanged(visible) {
            if (!LauncherController.visible) {
                item.close()
            }
        }
    }

    asynchronous: true
    sourceComponent: contextMenuComp

    function popup() {
        active = true
    }

    function close() {
        active = false
    }

    onStatusChanged: if (status == Loader.Ready) {
        item.popup()
    }
}
