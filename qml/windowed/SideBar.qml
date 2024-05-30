// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
    spacing: 10

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
        }
    }

    D.ToolButton {
        id: title

        Layout.alignment: Qt.AlignCenter
        KeyNavigation.down: computer
        KeyNavigation.up: setting
        KeyNavigation.tab: nextKeyTabTarget
        topPadding: 7
        bottomPadding: 5

        contentItem: ColumnLayout {
            spacing: 2
            D.DciIcon {
                width: 16
                height: 16
                name: isFreeSort ? categorizedIcon("freeSort") : categorizedIcon(CategorizedSortProxyModel.categoryType)
                palette: D.DTK.makeIconPalette(title.palette)
                theme: D.DTK.toColorType(title.palette.window)
                Layout.alignment: Qt.AlignHCenter
            }

            D.DciIcon {
                name: "arrow"
                width: 12
                height: 12
                palette: D.DTK.makeIconPalette(title.palette)
                theme: D.DTK.toColorType(title.palette.window)
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
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        Layout.alignment: Qt.AlignCenter
        focusPolicy: Qt.NoFocus
        // don't inherit window's windowText (it's has opacity of 0.7 in dtkgui.)
        palette.windowText: D.ColorSelector.textColor
        icon {
            width: 16
            height: 16
        }
        background: ItemBackground {
            button: btn
        }
    }

    SideBarButton {
        id: computer
        icon.name: "computer-symbolic"
        ToolTip.text: qsTr("Computer")
        KeyNavigation.down: images
        KeyNavigation.up: title
        onClicked: {
            DesktopIntegration.showUrl("computer:///")
        }
    }

    SideBarButton {
        id: images
        icon.name: "folder-pictures-symbolic"
        ToolTip.text: qsTr("Pictures")
        KeyNavigation.down: documents
        KeyNavigation.up: computer
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.PicturesLocation)
        }
    }

    SideBarButton {
        id: documents
        icon.name: "folder-documents-symbolic"
        ToolTip.text: qsTr("Documents")
        KeyNavigation.down: desktop
        KeyNavigation.up: images
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DocumentsLocation)
        }
    }

    SideBarButton {
        id: desktop
        icon.name: "user-desktop-symbolic"
        ToolTip.text: qsTr("Desktop")
        KeyNavigation.down: setting
        KeyNavigation.up: documents
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DesktopLocation)
        }
    }

    SideBarButton {
        id: setting
        icon.name: "setting"
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
        Layout.bottomMargin: 10
    }
}
