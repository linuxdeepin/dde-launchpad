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
        id: contextMenu
        Menu {
            MenuItem {
                text: qsTr("Open")
                onTriggered: {
                    launchApp(appItem.desktopId)
                }
            }
            MenuSeparator {}
            MenuItem {
                visible: isFavoriteItem && !hideFavoriteMenu
                height: visible ? implicitHeight : 0 // FIXME: seems this can cause some issue
                text: qsTr("Pin to Top")
            }
            MenuItem {
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
            MenuSeparator {}
            MenuItem { text: true ? qsTr("Send to desktop") : qsTr("Remove from desktop") }
            MenuItem { text: true ? qsTr("Send to dock") : qsTr("Remove from dock") }
            MenuSeparator {}
            MenuItem { text: true ? qsTr("Add to startup") : qsTr("Remove from startup") }
            MenuItem { text: qsTr("Use a proxy") }
            MenuItem {
                enabled: !AppsModel.appIsCompulsoryForDesktop(appItem.desktopId)
                text: qsTr("Uninstall")
            }

            onClosed: {
                root.closed()
                destroy()
            }
        }
    }

    asynchronous: true
    sourceComponent: contextMenu

    function popup() {
        active = true;
    }

    onStatusChanged: if (status == Loader.Ready) {
        item.popup()
    }
}
