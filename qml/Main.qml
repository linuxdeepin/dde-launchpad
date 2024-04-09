// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import org.deepin.launchpad.windowed 1.0

QtObject {
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
        if (DebugHelper.avoidLaunchApp) {
            DTK.sendSystemMessage("dde-launchpad (debug)",
                                  "clicked " + desktopId + " but won't attempt to launch it cause it's debug mode",
                                  "dialog-warning")
        } else {
            DesktopIntegration.launchByDesktopId(desktopId);
        }

        if (!DebugHelper.avoidHideWindow) {
            LauncherController.visible = false
        }
    }

    property var activeMenu: null
    property Component appContextMenuCom: AppItemMenu { }
    function showContextMenu(obj, model, folderIcons, isFavoriteItem, hideFavoriteMenu) {
        if (folderIcons) return

        const menu = appContextMenuCom.createObject(obj, {
            display: model.display,
            desktopId: model.desktopId,
            iconName: model.iconName,
            isFavoriteItem: isFavoriteItem,
            hideFavoriteMenu: hideFavoriteMenu,
            hideDisplayScalingMenu: false
        });
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

    function descaledRect(rect) {
        let ratio = Screen.devicePixelRatio
        return Qt.rect(rect.left / ratio, rect.top / ratio, rect.width / ratio, rect.height / ratio)
    }

    function updateWindowVisibilityAndPosition() {
        if (!LauncherController.visible) return;

        if (LauncherController.currentFrame === "WindowedFrame") {
//            root.visibility = Window.Windowed

            let width = windowedFrameSize.width
            let height = windowedFrameSize.height
            let x = 0
            let y = 0

            let dockGeometry = descaledRect(DesktopIntegration.dockGeometry)
            if (dockGeometry.width > 0 && dockGeometry.height > 0) {
//                console.log(114514, dockGeometry)
                switch (DesktopIntegration.dockPosition) {
                case Qt.DownArrow:
                    x = dockGeometry.left
                    y = (dockGeometry.top >= 0 ? dockGeometry.top : (Screen.height - dockGeometry.height)) - height - DesktopIntegration.dockSpacing
                    break
                case Qt.LeftArrow:
                    x = dockGeometry.right + DesktopIntegration.dockSpacing
                    y = (dockGeometry.top >= 0 ? dockGeometry.top : 0)
                    break
                case Qt.UpArrow:
                    x = dockGeometry.left
                    y = dockGeometry.bottom + DesktopIntegration.dockSpacing
                    break
                case Qt.RightArrow:
                    x = (dockGeometry.left >= 0 ? dockGeometry.left : (Screen.width - dockGeometry.width)) - width - DesktopIntegration.dockSpacing
                    y = dockGeometry.top
                    break
                }
            }

            windowedFrame.setGeometry(x, y, width, height)
            windowedFrame.requestActivate()
        } else {
//            root.visibility = Window.FullScreen
            if (DesktopIntegration.environmentVariable("DDE_CURRENT_COMPOSITOR") !== "TreeLand") {
                fullscreenFrame.setGeometry(Screen.virtualX, Screen.virtualY, Screen.width, Screen.height)
            } else {
                fullscreenFrame.showFullScreen()
            }
            fullscreenFrame.requestActivate()
        }
    }

    property Palette appTextColor: Palette {
        normal {
            common: Qt.rgba(0, 0, 0, 1)
            crystal: Qt.rgba(0, 0, 0, 1)
        }
        normalDark {
            common: Qt.rgba(1, 1, 1, 0.7)
            crystal: Qt.rgba(1, 1, 1, 0.7)
        }
    }

    readonly property size windowedFrameSize: Qt.size(610, 465)

    property var windowedFrame: ApplicationWindow {
        id: windowedFrameWindow
        objectName: "WindowedFrameApplicationWindow"
        title: "Windowed Launchpad"
        visible: LauncherController.visible && (LauncherController.currentFrame === "WindowedFrame")

        width: windowedFrameSize.width
        height: windowedFrameSize.height
        flags: {
            if (DebugHelper.useRegularWindow) return Qt.Window
            return (Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint | Qt.Tool)
        }
        StyledBehindWindowBlur {
            control: parent
            anchors.fill: parent
            function blendColorAlpha(fallback) {
                if (DesktopIntegration.opacity < 0)
                    return fallback
                return DesktopIntegration.opacity
            }
            blendColor: {
                if (valid) {
                    return DS.Style.control.selectColor(undefined,
                                                Qt.rgba(235 / 255.0, 235 / 255.0, 235 / 255.0, blendColorAlpha(0.6)),
                                                Qt.rgba(0, 0, 0, blendColorAlpha(85 / 255)))
                }
                return DS.Style.control.selectColor(undefined,
                                            DS.Style.behindWindowBlur.lightNoBlurColor,
                                            DS.Style.behindWindowBlur.darkNoBlurColor)
            }
        }
        InsideBoxBorder {
            anchors.fill: parent
            radius: windowedFrameWindow.DWindow.windowRadius
        }

        color: "transparent"

        DWindow.enabled: !DebugHelper.useRegularWindow
        DWindow.enableSystemResize: false
        DWindow.enableSystemMove: false

        onVisibleChanged: {
            updateWindowVisibilityAndPosition()
        }

        onActiveChanged: {
            if (!active && !DebugHelper.avoidHideWindow && (LauncherController.currentFrame === "WindowedFrame")) {
                // When composting is disabled, switching mode from fullscreen to windowed mode will cause window
                // activeChanged signal get emitted. We reused the delay timer here to avoid the window get hide
                // caused by that.
                // Issue: https://github.com/linuxdeepin/developer-center/issues/6818
                if (!LauncherController.shouldAvoidHideOrActive()) {
                    LauncherController.hideWithTimer()
                }
            }
        }

        Loader {
            anchors.fill: parent
            focus: true
            sourceComponent: WindowedFrame { }

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

    property var fullscreenFrame: ApplicationWindow {
        objectName: "FullscreenFrameApplicationWindow"
        title: "Fullscreen Launchpad"
        visible: LauncherController.visible && (LauncherController.currentFrame !== "WindowedFrame")

        width: Screen.width
        height: Screen.height
        // visibility: Window.FullScreen
        flags: {
            if (DebugHelper.useRegularWindow) return Qt.Window
            return (Qt.FramelessWindowHint | Qt.Tool)
        }

        DWindow.enabled: !DebugHelper.useRegularWindow
        DWindow.enableSystemResize: false
        DWindow.enableSystemMove: false
        // Fullscreen mode: always assume dark theme
        DWindow.themeType: ApplicationHelper.DarkType

        onVisibleChanged: {
            if (visible) {
                requestActivate()
            }
        }

        onActiveChanged: {
            if (!active && !DebugHelper.avoidHideWindow && (LauncherController.currentFrame === "FullscreenFrame")) {
                // When composting is disabled, switching mode from fullscreen to windowed mode will cause window
                // activeChanged signal get emitted. We reused the delay timer here to avoid the window get hide
                // caused by that.
                // Issue: https://github.com/linuxdeepin/developer-center/issues/6818
                if (!LauncherController.shouldAvoidHideOrActive()) {
                    LauncherController.hideWithTimer()
                }
            }
        }

        Loader {
            anchors.fill: parent
            focus: true
            source: "FullscreenFrame.qml"

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

    property var desktopIntegrationConn: Connections {
        target: DesktopIntegration
        function onDockGeometryChanged() {
            updateWindowVisibilityAndPosition()
        }
    }

    property var debugDialog: DialogWindow {
        id: debugDialog

        width: 400
        height: 400
        function open() {
            show()
        }

        Loader {
            active: debugDialog.visible

            source: 'DebugDialog.qml'
        }
    }

    property var uninstallDialog: DialogWindow {
        id: confirmUninstallDlg

        property string appId: ""
        property string appName: ""

        minimumWidth: layout.implicitWidth + 2 * DS.Style.dialogWindow.contentHMargin
        minimumHeight: layout.implicitHeight + DS.Style.dialogWindow.titleBarHeight
        maximumWidth: minimumWidth
        maximumHeight: minimumHeight
        ColumnLayout {
            id: layout
            spacing: 0
            Label {
                font: DTK.fontManager.t5
                text: qsTr("Are you sure you want to uninstall %1?").arg(confirmUninstallDlg.appName)
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignCenter
                Layout.margins: 10
            }
            RowLayout {
                spacing: 0
                Item {
                    Button {
                        id: cancelButton
                        text: qsTr("Cancel")
                        onClicked: {
                            confirmUninstallDlg.close()
                        }
                        anchors.centerIn: parent
                    }
                    Layout.fillWidth: true
                    Layout.preferredHeight: cancelButton.implicitHeight
                    Layout.topMargin: 20
                    Layout.bottomMargin: 20
                }
                Item {
                    WarningButton {
                        id: confirmButton
                        text: qsTr("Confirm")
                        onClicked: {
                            DesktopIntegration.uninstallApp(confirmUninstallDlg.appId)
                            confirmUninstallDlg.close()
                        }
                        anchors.centerIn: parent
                    }
                    Layout.fillWidth: true
                    Layout.preferredHeight: confirmButton.implicitHeight
                    Layout.topMargin: 20
                    Layout.bottomMargin: 20
                }
            }
        }
    }
}
