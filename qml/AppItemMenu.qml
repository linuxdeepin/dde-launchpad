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
            margins: isFullscreen ? dockSpacing : 0
            modal: true

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
                    LauncherController.visible = false
                    confirmUninstallDlg.appName = root.display
                    confirmUninstallDlg.appId = root.desktopId
                    confirmUninstallDlg.icon = root.iconName
                    confirmUninstallDlg.show()
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
