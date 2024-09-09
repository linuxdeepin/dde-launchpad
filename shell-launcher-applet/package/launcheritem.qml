// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.deepin.dtk 1.0

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.ds.dock 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import org.deepin.launchpad.windowed 1.0

AppletItem {
    id: launcher
    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: 12
    // 1:4 the distance between app : dock height; get width/height≈0.8
    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : Panel.rootObject.dockItemMaxSize * 0.8
    implicitHeight: useColumnLayout ? Panel.rootObject.dockItemMaxSize * 0.8 : Panel.rootObject.dockSize

    Connections {
        target: Panel.rootObject
        function onDockCenterPartPosChanged()
        {
            updateLaunchpadPos()
        }
    }

    property point itemPos: Qt.point(0, 0)
    function updateItemPos()
    {
        var lX = icon.mapToItem(null, 0, 0).x
        var lY = icon.mapToItem(null, 0, 0).y
        launcher.itemPos = Qt.point(lX, lY)
    }
    function updateLaunchpadPos()
    {
        updateItemPos()
        var launchpad = DS.applet("org.deepin.ds.launchpad")
        if (!launchpad || !launchpad.rootObject)
            return

        launchpad.rootObject.windowedPos = launcher.itemPos
    }
    Component.onCompleted: {
        updateLaunchpadPos()
    }

    property var activeMenu: null
    property Component appContextMenuCom: AppItemMenu { }
    function showContextMenu(obj, model, additionalProps = {}) {
        if (!obj || !obj.Window.window) {
            console.log("obj or obj.Window.window is null")
            return
        }
        closeContextMenu()

        const menu = appContextMenuCom.createObject(obj.Window.window.contentItem, Object.assign({
            display: model.display,
            desktopId: model.desktopId,
            iconName: model.iconName,
            isFavoriteItem: false,
            hideFavoriteMenu: true,
            hideDisplayScalingMenu: false,
            hideMoveToTopMenu: true
        }, additionalProps));
        menu.closed.connect(menu.destroy)
        menu.popup();

        activeMenu = menu
    }

    function closeContextMenu() {
        if (activeMenu) {
            activeMenu.close()
            activeMenu = null
        }
    }

    function getCategoryName(section) {
        switch (Number(section)) {
        case AppItem.Internet:
            return qsTr("Internet");
        case AppItem.Chat:
            return qsTr("Chat");
        case AppItem.Music:
            return qsTr("Music");
        case AppItem.Video:
            return qsTr("Video");
        case AppItem.Graphics:
            return qsTr("Graphics");
        case AppItem.Game:
            return qsTr("Game");
        case AppItem.Office:
            return qsTr("Office");
        case AppItem.Reading:
            return qsTr("Reading");
        case AppItem.Development:
            return qsTr("Development");
        case AppItem.System:
            return qsTr("System");
        default:
            return qsTr("Others");
        }
    }

    function launchApp(desktopId) {
        DesktopIntegration.launchByDesktopId(desktopId);
    }

    PanelToolTip {
        id: toolTip
        text: qsTr("launchpad")
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    property var fullscreenFrame: ApplicationWindow {
        objectName: "FullscreenFrameApplicationWindow"
        title: "Fullscreen Launchpad"
        visible: LauncherController.visible && (LauncherController.currentFrame !== "WindowedFrame")
        // Set transparent on kwin will cause abnormal rounded corners in FolderPopup, Bug: 10219
        color: DesktopIntegration.isTreeLand() ? "transparent" : undefined
        transientParent: null

        DLayerShellWindow.anchors: DLayerShellWindow.AnchorBottom | DLayerShellWindow.AnchorTop | DLayerShellWindow.AnchorLeft | DLayerShellWindow.AnchorRight
        DLayerShellWindow.layer: DLayerShellWindow.LayerTop
        DLayerShellWindow.keyboardInteractivity: DLayerShellWindow.KeyboardInteractivityOnDemand
        DLayerShellWindow.exclusionZone: -1
        DLayerShellWindow.scope: "dde-shell/launchpad"

        // visibility: Window.FullScreen
        flags: {
            if (DebugHelper.useRegularWindow) return Qt.Window
            return (Qt.FramelessWindowHint | Qt.Tool)
        }

        DWindow.enabled: !DebugHelper.useRegularWindow
        DWindow.windowRadius: 0
        DWindow.enableSystemResize: false
        DWindow.enableSystemMove: false
        // Fullscreen mode: always assume dark theme
        DWindow.themeType: ApplicationHelper.DarkType
        DWindow.windowStartUpEffect: PlatformHandle.EffectOut

        onVisibleChanged: {
            if (visible) {
                requestActivate()
            }
        }

        onActiveChanged: {
            if (LauncherController.currentFrame !== "FullscreenFrame") {
                return
            }
            if (active) {
                LauncherController.cancelHide()
                return;
            }
            if (!active && !DebugHelper.avoidHideWindow) {
                LauncherController.hideWithTimer()
            }
        }

        Loader {
            anchors.fill: parent
            focus: true
            sourceComponent: FullscreenFrame {}

            Label {
                visible: DebugHelper.qtDebugEnabled
                z: 999

                anchors.right: parent.right
                anchors.bottom: parent.bottom
                text: "/ / Under Construction / /"

                background: Rectangle {
                    color: Qt.rgba(1, 1, 0, 0.5)
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: { debugDialog.open() }
                }
            }
        }
    }

    PanelPopup {
        id: windowedModeLauncher

        property bool visibility: LauncherController.visible && (LauncherController.currentFrame === "WindowedFrame")

        width: 610
        height: 480
        popupX: launcher.itemPos.x
        popupY: launcher.itemPos.y

        WindowedFrame {
            anchors.fill: parent
        }

        onVisibilityChanged: function() {
            if (visibility) {
                if (!windowedModeLauncher.visible) {
                    windowedModeLauncher.open()
                }
            } else {
                windowedModeLauncher.close()
            }
        }
    }

    D.DciIcon {
        id: icon
        anchors.centerIn: parent
        name: Applet.iconName
        scale: Panel.rootObject.dockItemMaxSize * 9 / 14 / Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE
        // 9:14 (iconSize/dockHeight)
        sourceSize: Qt.size(Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE, Dock.MAX_DOCK_TASKMANAGER_ICON_SIZE)
        onXChanged: updateLaunchpadPos()
        onYChanged: updateLaunchpadPos()
        Timer {
            id: toolTipShowTimer
            interval: 50
            onTriggered: {
                var point = Applet.rootObject.mapToItem(null, Applet.rootObject.width / 2, Applet.rootObject.height / 2)
                toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                toolTip.open()
            }
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                LauncherController.visible = !LauncherController.visible
                toolTip.close()
            }
        }
        HoverHandler {
            onHoveredChanged: {
                if (hovered) {
                    toolTipShowTimer.start()
                } else {
                    if (toolTipShowTimer.running) {
                        toolTipShowTimer.stop()
                    }

                    toolTip.close()
                }
            }
        }
    }
}
