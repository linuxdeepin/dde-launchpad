// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.platform 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.dtk.style 1.0 as DS

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

ColumnLayout {
    spacing: Helper.pixelAligned(10, Screen.devicePixelRatio)

    property bool isFreeSort: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.FreeCategory
    property Item keyTabTarget: title
    property Item nextKeyTabTarget

    signal switchToFreeSort(bool freeSort)

    function categorizedIcon(categoryType) {
        switch (categoryType) {
        case CategorizedSortProxyModel.DDECategory: return "classify";
        case CategorizedSortProxyModel.Alphabetary: return "name";
        default: return "arrange"
        }
    }

    Component {
        id: categorizedCom
        D.Menu {
            id: categorizedMenu

            D.MenuItem {
                text: qsTr("Free sorting")
                icon.name: categorizedIcon("freeSort")
                display: D.IconLabel.IconBesideText
                checked: isFreeSort
                onTriggered: {
                    if (!isFreeSort) {
                        isFreeSort = true
                        switchToFreeSort(true)
                        CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.FreeCategory
                    }
                }
            }

            D.MenuItem {
                text: qsTr("Sort by category")
                icon.name: categorizedIcon(CategorizedSortProxyModel.DDECategory)
                display: D.IconLabel.IconBesideText
                checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.DDECategory
                onTriggered: {
                    isFreeSort = false
                    switchToFreeSort(false)
                    CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.DDECategory
                }
            }

            D.MenuItem {
                text: qsTr("Sort by name")
                icon.name: categorizedIcon(CategorizedSortProxyModel.Alphabetary)
                display: D.IconLabel.IconBesideText
                checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary
                onTriggered: {
                    isFreeSort = false
                    switchToFreeSort(false)
                    CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.Alphabetary
                }
            }

            Connections {
                target: LauncherController
                function onVisibleChanged(visible) {
                    if (!visible) {
                        categorizedMenu.close()
                    }
                }
            }
        }
    }

    D.ToolButton {
        id: title

        Layout.alignment: Qt.AlignCenter
        KeyNavigation.down: computer
        KeyNavigation.up: setting
        KeyNavigation.tab: nextKeyTabTarget
        topPadding: Helper.pixelAligned(7, Screen.devicePixelRatio)
        bottomPadding: Helper.pixelAligned(5, Screen.devicePixelRatio)
        ToolTip.visible: hovered
        ToolTip.delay: 500
        ToolTip.text: qsTr("Sorting Mode")

        contentItem: ColumnLayout {
            spacing: 2
            PaintedDciIcon {
                sourceSize: Qt.size(16, 16)
                width: 16
                height: 16
                name: isFreeSort ? categorizedIcon("freeSort") : categorizedIcon(CategorizedSortProxyModel.categoryType)
                foreground: title.palette.windowText
                Layout.alignment: Qt.AlignHCenter
            }

            PaintedDciIcon {
                name: "arrow"
                sourceSize: Qt.size(12, 12)
                width: 12
                height: 12
                foreground: title.palette.windowText
                Layout.alignment: Qt.AlignHCenter
            }
        }

        background: ItemBackground {
            button: title
        }

        onClicked: {
            const menu = categorizedCom.createObject(this);
            menu.popup(this, Qt.point(title.x, title.y));
        }
    }

    Item {
        Layout.fillHeight: true
    }

    component SideBarButton: D.ActionButton {
        id: btn
        property alias iconSource: paintedIcon.name
        ToolTip.visible: hovered
        ToolTip.delay: 500
        Layout.alignment: Qt.AlignCenter
        focusPolicy: Qt.NoFocus
        // don't inherit window's windowText (it's has opacity of 0.7 in dtkgui.)
        palette.windowText: D.ColorSelector.textColor
        contentItem: PaintedDciIcon {
            id: paintedIcon
            width: 16
            height: 16
            sourceSize: Qt.size(16, 16)
            foreground: D.ColorSelector.textColor
        }
        background: ItemBackground {
            button: btn
        }
    }

    SideBarButton {
        id: computer
        iconSource: "computer-symbolic"
        ToolTip.text: qsTr("Computer")
        KeyNavigation.down: images
        KeyNavigation.up: title
        onClicked: {
            DesktopIntegration.showUrl("computer:///")
        }
    }

    SideBarButton {
        id: images
        iconSource: "folder-pictures-symbolic"
        ToolTip.text: qsTr("Pictures")
        KeyNavigation.down: documents
        KeyNavigation.up: computer
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.PicturesLocation)
        }
    }

    SideBarButton {
        id: documents
        iconSource: "folder-documents-symbolic"
        ToolTip.text: qsTr("Documents")
        KeyNavigation.down: desktop
        KeyNavigation.up: images
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DocumentsLocation)
        }
    }

    SideBarButton {
        id: desktop
        iconSource: "user-desktop-symbolic"
        ToolTip.text: qsTr("Desktop")
        KeyNavigation.down: setting
        KeyNavigation.up: documents
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DesktopLocation)
        }
    }

    SideBarButton {
        id: setting
        iconSource: "setting"
        ToolTip.text: qsTr("Control Center")
        KeyNavigation.down: title
        KeyNavigation.up: desktop
        onClicked: {
            DesktopIntegration.openSystemSettings();
        }
    }

    Item {
        Layout.fillHeight: true
    }

    Item {
        height: title.height
        Layout.bottomMargin: Helper.pixelAligned(10, Screen.devicePixelRatio)
    }
}
