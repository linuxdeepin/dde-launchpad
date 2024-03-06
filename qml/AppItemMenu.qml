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
    property bool hideDisplayScalingMenu

    signal closed()

    Component {
        id: contextMenuComp

        Menu {
            id: contextMenu

            MenuItem {
                text: qsTr("Open")
                onTriggered: {
                    launchApp(root.desktopId)
                }
            }
            MenuSeparator {}
            MenuItem {
                id: pinToTopMenu
                visible: isFavoriteItem && !hideFavoriteMenu
                height: visible ? implicitHeight : 0 // FIXME: seems this can cause some issue
                text: qsTr("Pin to Top")
                onTriggered: {
                    FavoritedProxyModel.pinToTop(root.desktopId)
                }
            }
            MenuItem {
                id: addOrRemoveFavMenu
                visible: !hideFavoriteMenu
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
                text: DesktopIntegration.isAutoStart(root.desktopId) ? qsTr("Remove from startup") : qsTr("Add to startup")
                onTriggered: {
                    DesktopIntegration.setAutoStart(root.desktopId, !DesktopIntegration.isAutoStart(root.desktopId))
                }
            }
            MenuItem {
                visible: false
                text: qsTr("Use a proxy")
                height: visible ? implicitHeight : 0 // FIXME: same as above
            }
            MenuItem {
                visible: !hideDisplayScalingMenu
                height: visible ? implicitHeight : 0 // FIXME: same as above
                checkable: true
                checked: Math.abs(DesktopIntegration.scaleFactor(root.desktopId) - 1.0) < 0.0001
                text: qsTr("Disable display scaling")
                onTriggered: {
                    DesktopIntegration.setScaleFactor(root.desktopId, checked ? 1 : 0)
                }
            }
            MenuItem {
                enabled: !DesktopIntegration.appIsCompulsoryForDesktop(root.desktopId)
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
        onVisibleChanged: function () {
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

    onStatusChanged: if (status == Loader.Ready) {
        item.popup()
    }
}
