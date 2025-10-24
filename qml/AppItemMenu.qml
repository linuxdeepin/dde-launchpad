// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Loader {
    id: root

    property string display
    property string desktopId
    property string iconName
    property bool isFavoriteItem
    property bool hideFavoriteMenu
    property bool hideMoveToTopMenu
    property bool hideDisplayScalingMenu
    readonly property bool isFullscreen: LauncherController.currentFrame === "FullscreenFrame"
    readonly property bool isHorizontalDock: DesktopIntegration.dockPosition === Qt.UpArrow || DesktopIntegration.dockPosition === Qt.DownArrow
    readonly property int dockSpacing: (isHorizontalDock ? DesktopIntegration.dockGeometry.height : DesktopIntegration.dockGeometry.width) / Screen.devicePixelRatio
    signal closed()

    Component {
        id: contextMenuComp

        Menu {
            id: contextMenu

            topMargin: isFullscreen && DesktopIntegration.dockPosition === Qt.UpArrow ? dockSpacing : 0
            bottomMargin: isFullscreen && DesktopIntegration.dockPosition === Qt.DownArrow ? dockSpacing : 0
            leftMargin: isFullscreen && DesktopIntegration.dockPosition === Qt.LeftArrow ? dockSpacing : 0
            rightMargin: isFullscreen && DesktopIntegration.dockPosition === Qt.RightArrow ? dockSpacing : 0
            modal: true

            // 在全屏模式下，右键菜单适配系统深浅模式
            // 通过设置palette来实现主题适配
            palette: DTK.palette

            MenuItem {
                text: qsTr("Open")
                enabled: !root.desktopId.startsWith("internal/folders/")
                onTriggered: {
                    launchApp(root.desktopId)
                }
            }
            MenuSeparator {}
            MenuItem {
                id: pinToTopMenu
                enabled: false
                visible: isFavoriteItem && !hideFavoriteMenu
                height: visible ? implicitHeight : 0 // FIXME: seems this can cause some issue
                text: qsTr("Pin to Top")
                onTriggered: {
                    FavoritedProxyModel.pinToTop(root.desktopId)
                }
            }
            MenuItem {
                id: moveToTopMenu
                visible: !hideMoveToTopMenu
                enabled: visible
                height: visible ? implicitHeight : 0
                text: qsTr("Move to Top")
                onTriggered: {
                    ItemArrangementProxyModel.bringToFront(root.desktopId)
                }
            }
            MenuItem {
                id: addOrRemoveFavMenu
                visible: !hideFavoriteMenu
                enabled: false
                height: visible ? implicitHeight : 0 // FIXME: same as above
                text: FavoritedProxyModel.exists(root.desktopId) ? qsTr("Remove from favorites") : qsTr("Add to favorites")
                onTriggered: {
                    if (FavoritedProxyModel.exists(root.desktopId)) {
                        FavoritedProxyModel.removeFavorite(root.desktopId);
                    } else {
                        FavoritedProxyModel.addFavorite(root.desktopId);
                    }
                }
            }
            MenuSeparator {
                visible: pinToTopMenu.visible && addOrRemoveFavMenu.visible
                height: visible ? implicitHeight : 0 // FIXME: same as above
            }
            MenuItem {
                enabled: !root.desktopId.startsWith("internal/folders/")
                text: DesktopIntegration.isOnDesktop(root.desktopId) ? qsTr("Remove from desktop") : qsTr("Send to desktop")
                onTriggered: {
                    if (DesktopIntegration.isOnDesktop(root.desktopId)) {
                        DesktopIntegration.removeFromDesktop(root.desktopId);
                    } else {
                        DesktopIntegration.sendToDesktop(root.desktopId);
                    }
                }
            }
            MenuItem {
                enabled: !root.desktopId.startsWith("internal/")
                text: DesktopIntegration.isDockedApp(root.desktopId) ? qsTr("Remove from dock") : qsTr("Send to dock")
                onTriggered: {
                    if (DesktopIntegration.isDockedApp(root.desktopId)) {
                        DesktopIntegration.removeFromDock(root.desktopId);
                    } else {
                        DesktopIntegration.sendToDock(root.desktopId);
                    }
                }
            }
            MenuSeparator {}
            MenuItem {
                enabled: !root.desktopId.startsWith("internal/folders/")
                text: DesktopIntegration.isAutoStart(root.desktopId) ? qsTr("Remove from startup") : qsTr("Add to startup")
                onTriggered: {
                    DesktopIntegration.setAutoStart(root.desktopId, !DesktopIntegration.isAutoStart(root.desktopId))
                }
            }
            MenuItem {
                visible: false
                enabled: false
                text: qsTr("Use a proxy")
                height: visible ? implicitHeight : 0 // FIXME: same as above
            }
            MenuItem {
                visible: !hideDisplayScalingMenu
                enabled: !root.desktopId.startsWith("internal/folders/")
                height: visible ? implicitHeight : 0 // FIXME: same as above
                checkable: true
                checked: DesktopIntegration.disableScale(root.desktopId)
                text: qsTr("Disable display scaling")
                onTriggered: {
                    DesktopIntegration.setDisableScale(root.desktopId, checked ? true : false)
                }
            }
            MenuItem {
                enabled: !root.desktopId.startsWith("internal/folders/") && !DesktopIntegration.appIsCompulsoryForDesktop(root.desktopId)
                text: qsTr("Uninstall")
                onTriggered: {
                    if (LauncherController.currentFrame !== "FullscreenFrame") {
                        LauncherController.setAvoidHide(true)
                        LauncherController.visible = false
                    } else {
                        LauncherController.setAvoidHide(false)
                    }
                    if (!DesktopIntegration.shouldSkipConfirmUninstallDialog(root.desktopId)) {
                        confirmUninstallDlg.appName = root.display
                        confirmUninstallDlg.appId = root.desktopId
                        confirmUninstallDlg.icon = root.iconName
                        confirmUninstallDlg.show()
                    } else {
                        DesktopIntegration.uninstallApp(root.desktopId)
                    }
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
        active = true;
    }

    function close() {
        active = false
    }

    onStatusChanged: if (status == Loader.Ready) {
        item.popup()
    }
}
