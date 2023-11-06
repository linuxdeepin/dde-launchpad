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

ApplicationWindow {
    id: root

    property bool isMenuShown: false

//    title: activeFocusItem + " " + (activeFocusItem ? activeFocusItem.Accessible.name : "")
    width: 780
    height: 600
    visible: LauncherController.visible
    flags: {
        if (DebugHelper.useRegularWindow) {
            return Qt.Window
        }
        if (LauncherController.currentFrame === "WindowedFrame") {
            return (Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint | Qt.Tool) // X11BypassWindowManagerHint
        } else {
            return (Qt.FramelessWindowHint | Qt.Tool)
        }
    }
    DWindow.enabled: !DebugHelper.useRegularWindow
    DWindow.enableBlurWindow: true
    DWindow.enableSystemResize: false
    DWindow.enableSystemMove: false

    onVisibleChanged: {
        updateWindowVisibilityAndPosition()
    }

    onActiveChanged: {
        if (!active && !isMenuShown && !DebugHelper.avoidHideWindow) {
            LauncherController.hideWithTimer()
        }
    }

    function descaledRect(rect) {
        let ratio = Screen.devicePixelRatio
        return Qt.rect(rect.left / ratio, rect.top / ratio, rect.width / ratio, rect.height / ratio)
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
        menu.closed.connect(function() {
            root.isMenuShown = false
            root.requestActivate()
        });
        menu.popup();
        root.isMenuShown = true
    }

    function updateWindowVisibilityAndPosition() {
        if (!root.visible) return;

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
            root.setGeometry(x, y, width, height)
        } else {
//            root.visibility = Window.FullScreen
            // Fullscreen mode: always assume dark theme
            ApplicationHelper.setPaletteType(ApplicationHelper.DarkType)
            root.setGeometry(Screen.virtualX, Screen.virtualY, Screen.width, Screen.height)
        }

        root.requestActivate()
    }

    Connections {
        target: DesktopIntegration
        function onDockGeometryChanged() {
            updateWindowVisibilityAndPosition()
        }
    }

    Loader {
        id: frameLoader
        anchors.fill: parent
        focus: true
        source: LauncherController.currentFrame + ".qml"
        onSourceChanged: {
            updateWindowVisibilityAndPosition();
        }

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

    Dialog {
        id: debugDialog
        modal: true

        standardButtons: Dialog.Close

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Loader {
            active: debugDialog.visible

            source: 'DebugDialog.qml'
        }
    }

    DialogWindow {
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
