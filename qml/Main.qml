// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

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

    function showContextMenu(obj, model, folderIcons, isFavoriteItem, hideFavoriteMenu) {
        if (folderIcons) return
        const component = Qt.createComponent(Qt.resolvedUrl("./AppItemMenu.qml"))
        const menu = component.createObject(obj, {
            appItem: model,
            isFavoriteItem: isFavoriteItem,
            hideFavoriteMenu: hideFavoriteMenu
        });
        menu.popup();
    }

    function descaledRect(rect) {
        let ratio = Screen.devicePixelRatio
        return Qt.rect(rect.left / ratio, rect.top / ratio, rect.width / ratio, rect.height / ratio)
    }

    function updateWindowVisibilityAndPosition() {
        if (!LauncherController.visible) return;

        if (LauncherController.currentFrame === "WindowedFrame") {
//            root.visibility = Window.Windowed
            let width = 780
            let height = 600
            let x = 0
            let y = 0

            let dockGeometry = descaledRect(DesktopIntegration.dockGeometry)
            if (dockGeometry.width > 0 && dockGeometry.height > 0) {
//                console.log(114514, dockGeometry)
                switch (DesktopIntegration.dockPosition) {
                case Qt.DownArrow:
                    x = dockGeometry.left
                    y = (dockGeometry.top >= 0 ? dockGeometry.top : (Screen.height - dockGeometry.height)) - height
                    break
                case Qt.LeftArrow:
                    x = dockGeometry.width
                    y = (dockGeometry.top >= 0 ? dockGeometry.top : 0)
                    break
                case Qt.UpArrow:
                    x = dockGeometry.left
                    y = dockGeometry.height
                    break
                case Qt.RightArrow:
                    x = (dockGeometry.left >= 0 ? dockGeometry.left : (Screen.width - dockGeometry.width)) - width
                    y = dockGeometry.top
                    break
                }
            }

            // Window mode: follow system theme
            ApplicationHelper.setPaletteType(ApplicationHelper.UnknownType)
            windowedFrame.setGeometry(x, y, width, height)
            windowedFrame.requestActivate()
        } else {
//            root.visibility = Window.FullScreen
            // Fullscreen mode: always assume dark theme
            ApplicationHelper.setPaletteType(ApplicationHelper.DarkType)
            fullscreenFrame.setGeometry(Screen.virtualX, Screen.virtualY, Screen.width, Screen.height)
            fullscreenFrame.requestActivate()
        }
    }

    property var windowedFrame: ApplicationWindow {
        visible: LauncherController.visible && (LauncherController.currentFrame === "WindowedFrame")

        width: 780
        height: 600
        flags: {
            if (DebugHelper.useRegularWindow) return Qt.Window
            return (Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint | Qt.Tool)
        }

        DWindow.enabled: !DebugHelper.useRegularWindow
        DWindow.enableBlurWindow: true
        DWindow.enableSystemResize: false
        DWindow.enableSystemMove: false

        onVisibleChanged: {
            updateWindowVisibilityAndPosition()
        }

        onActiveChanged: {
            if (!active && !DebugHelper.avoidHideWindow && (LauncherController.currentFrame === "WindowedFrame")) {
                LauncherController.hideWithTimer()
            }
        }

        Loader {
            anchors.fill: parent
            focus: true
            source: "WindowedFrame.qml"

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

        width: 400

        ColumnLayout {
            width: parent.width
            Label {
                Layout.alignment: Qt.AlignHCenter
                font: DTK.fontManager.t5
                text: qsTr("Are you sure you want to uninstall %1?").arg(confirmUninstallDlg.appName)
            }
            RowLayout {
                Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                Layout.bottomMargin: 10
                Layout.topMargin: 10
                Layout.fillWidth: true
                Button {
                    text: qsTr("Cancel")
                    Layout.preferredWidth: 175
                    onClicked: {
                        confirmUninstallDlg.close()
                    }
                }
                Item {Layout.fillWidth: true}
                WarningButton {
                    text: qsTr("Confirm")
                    Layout.preferredWidth: 175
                    Layout.alignment: Qt.AlignRight
                    onClicked: {
                        DesktopIntegration.uninstallApp(confirmUninstallDlg.appId)
                        confirmUninstallDlg.close()
                    }
                }
            }
        }
    }
}
