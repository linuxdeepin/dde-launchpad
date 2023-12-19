// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

Loader {
    id: root

    property var appItem
    property bool isFavoriteItem
    property bool hideFavoriteMenu

    signal closed()

    Component {
        id: contextMenuComp

        Menu {
            id: contextMenu

            MenuItem {
                text: qsTr("Open")
                onTriggered: {
                    launchApp(appItem.desktopId)
                }
            }
            MenuSeparator {}
            MenuItem {
                id: pinToTopMenu
                visible: isFavoriteItem && !hideFavoriteMenu
                height: visible ? implicitHeight : 0 // FIXME: seems this can cause some issue
                text: qsTr("Pin to Top")
                onTriggered: {
                    FavoritedProxyModel.pinToTop(appItem.desktopId)
                }
            }
            MenuItem {
                id: addOrRemoveFavMenu
                visible: !hideFavoriteMenu
                height: visible ? implicitHeight : 0 // FIXME: same as above
                text: FavoritedProxyModel.exists(appItem.desktopId) ? qsTr("Remove from favorites") : qsTr("Add to favorites")
                onTriggered: {
                    if (FavoritedProxyModel.exists(appItem.desktopId)) {
                        FavoritedProxyModel.removeFavorite(appItem.desktopId);
                    } else {
                        FavoritedProxyModel.addFavorite(appItem.desktopId);
                    }
                }
            }
            MenuSeparator {
                visible: pinToTopMenu.visible && addOrRemoveFavMenu.visible
                height: visible ? implicitHeight : 0 // FIXME: same as above
            }
            MenuItem {
                text: DesktopIntegration.isOnDesktop(appItem.desktopId) ? qsTr("Remove from desktop") : qsTr("Send to desktop")
                onTriggered: {
                    if (DesktopIntegration.isOnDesktop(appItem.desktopId)) {
                        DesktopIntegration.removeFromDesktop(appItem.desktopId);
                    } else {
                        DesktopIntegration.sendToDesktop(appItem.desktopId);
                    }
                }
            }
            MenuItem {
                text: DesktopIntegration.isDockedApp(appItem.desktopId) ? qsTr("Remove from dock") : qsTr("Send to dock")
                onTriggered: {
                    if (DesktopIntegration.isDockedApp(appItem.desktopId)) {
                        DesktopIntegration.removeFromDock(appItem.desktopId);
                    } else {
                        DesktopIntegration.sendToDock(appItem.desktopId);
                    }
                }
            }
            MenuSeparator {}
            MenuItem {
                text: DesktopIntegration.isAutoStart(appItem.desktopId) ? qsTr("Remove from startup") : qsTr("Add to startup")
                onTriggered: {
                    DesktopIntegration.setAutoStart(appItem.desktopId, !DesktopIntegration.isAutoStart(appItem.desktopId))
                }
            }
            MenuItem { text: qsTr("Use a proxy") }
            MenuItem {
                enabled: !DesktopIntegration.appIsCompulsoryForDesktop(appItem.desktopId)
                text: qsTr("Uninstall")
                onTriggered: {
                    LauncherController.visible = false
                    confirmUninstallDlg.appName = appItem.display
                    confirmUninstallDlg.appId = appItem.desktopId
                    confirmUninstallDlg.show()
                }
            }

            onClosed: {
                root.closed()
                destroy()
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
