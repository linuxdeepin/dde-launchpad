// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0
import org.deepin.ds 1.0
import org.deepin.dtk.style 1.0 as DStyle

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

    function decrementPageIndex(pages) {
        if (pages.currentIndex === 0 && pages.count > 1) {
            // pages.setCurrentIndex(pages.count - 1)
        } else {
            pages.decrementCurrentIndex()
        }

        closeContextMenu()
    }

    function incrementPageIndex(pages) {
        if (pages.currentIndex === pages.count - 1 && pages.count > 1) {
            // pages.setCurrentIndex(0)
        } else {
            pages.incrementCurrentIndex()
        }

        closeContextMenu()
    }

    function descaledRect(rect) {
        let ratio = Screen.devicePixelRatio
        return Qt.rect(rect.left / ratio, rect.top / ratio, rect.width / ratio, rect.height / ratio)
    }
    function descaledPos(pos) {
        let ratio = Screen.devicePixelRatio
        return Qt.point(pos.x / ratio, pos.y / ratio)
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
            let descaledWindowdPos = windowedPos
            if (dockGeometry.width > 0 || dockGeometry.height > 0) {
                let dockSpacing = DesktopIntegration.dockSpacing
                switch (DesktopIntegration.dockPosition) {
                case Qt.DownArrow:
                    var sidebarX = windowedFrameImpl.item.getHorizontalCoordinatesOfSideBar()
                    x = descaledWindowdPos.x > 0 ? descaledWindowdPos.x - sidebarX : dockGeometry.left + dockSpacing
                    y = (dockGeometry.top >= 0 ? dockGeometry.top : (Screen.height - dockGeometry.height)) - height - dockSpacing
                    break
                case Qt.LeftArrow:
                    x = dockGeometry.right + dockSpacing
                    y = (dockGeometry.top >= 0 ? dockGeometry.top : 0) + dockSpacing
                    break
                case Qt.UpArrow:
                    x = dockGeometry.left + dockSpacing
                    y = dockGeometry.bottom + dockSpacing
                    break
                case Qt.RightArrow:
                    x = (dockGeometry.left >= 0 ? dockGeometry.left : (Screen.width - dockGeometry.width)) - width - dockSpacing
                    y = dockGeometry.top + dockSpacing
                    break
                }
            } else {
                console.warn("Invalid dock geometry", dockGeometry)
            }

            windowedFrame.setGeometry(x, y, width, height)
            windowedFrame.requestActivate()
        } else {
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

    readonly property size windowedFrameSize: Qt.size(610, 480)

    // update by caller.
    property point windowedPos: Qt.point(0, 0)
    property var windowedFrame: ApplicationWindow {
        id: windowedFrameWindow
        objectName: "WindowedFrameApplicationWindow"
        title: "Windowed Launchpad"
        visible: LauncherController.visible && (LauncherController.currentFrame === "WindowedFrame")

        width: windowedFrameSize.width
        height: windowedFrameSize.height
        maximumWidth: width
        minimumWidth: width
        minimumHeight: height
        maximumHeight: height
        ColorSelector.family: Palette.CrystalColor
        flags: {
            if (DebugHelper.useRegularWindow) return Qt.Window
            return ( Qt.FramelessWindowHint | Qt.Tool)
        }

        function blendColorAlpha(fallback) {
            var appearance = DS.applet("org.deepin.ds.dde-appearance")
            if (!appearance || appearance.opacity < 0)
                return fallback
            return appearance.opacity
        }

        StyledBehindWindowBlur {
            control: parent
            anchors.fill: parent
            blendColor: {
                if (valid) {
                    return DStyle.Style.control.selectColor(undefined,
                                                Qt.rgba(235 / 255.0, 235 / 255.0, 235 / 255.0, windowedFrameWindow.blendColorAlpha(0.6)),
                                                Qt.rgba(0, 0, 0, windowedFrameWindow.blendColorAlpha(85 / 255)))
                }
                return DStyle.Style.control.selectColor(undefined,
                                            DStyle.Style.behindWindowBlur.lightNoBlurColor,
                                            DStyle.Style.behindWindowBlur.darkNoBlurColor)
            }
        }
        InsideBoxBorder {
            anchors.fill: parent
            radius: WindowManagerHelper.hasComposite ? windowedFrameWindow.DWindow.windowRadius : 0
            color: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, windowedFrameWindow.blendColorAlpha(0.6) - 0.05)
        }

        color: "transparent"

        DWindow.enabled: !DebugHelper.useRegularWindow
        DWindow.windowRadius: 12
        DWindow.enableSystemResize: false
        DWindow.enableSystemMove: false
        DWindow.enableBlurWindow: true
        DWindow.borderColor: DTK.themeType === ApplicationHelper.DarkType ? Qt.rgba(0, 0, 0, windowedFrameWindow.blendColorAlpha(0.6) + 10 / 255) : Qt.rgba(0, 0, 0, 0.15)

        onVisibleChanged: {
            updateWindowVisibilityAndPosition()
        }

        onActiveChanged: {
            if (!active && !DebugHelper.avoidHideWindow && (LauncherController.currentFrame === "WindowedFrame")) {
                LauncherController.hideWithTimer()
            }
        }

        Loader {
            id: windowedFrameImpl
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

        DLayerShellWindow.anchors: DLayerShellWindow.AnchorBottom | DLayerShellWindow.AnchorTop | DLayerShellWindow.AnchorLeft | DLayerShellWindow.AnchorRight
        DLayerShellWindow.layer: DLayerShellWindow.LayerTop

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
                LauncherController.hideWithTimer()
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

        DLayerShellWindow.anchors: DLayerShellWindow.AnchorNone

        minimumWidth: layout.implicitWidth + 2 * DStyle.Style.dialogWindow.contentHMargin
        minimumHeight: layout.implicitHeight + DStyle.Style.dialogWindow.titleBarHeight
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
