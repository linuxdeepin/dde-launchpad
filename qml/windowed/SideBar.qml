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
    Layout.fillWidth: false
    Layout.fillHeight: true
    Layout.margins: 10
    spacing: 10

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
                icon.name: categorizedIcon("todo")
                display: D.IconLabel.IconBesideText
                enabled: false
                onTriggered: { }
            }

            D.MenuItem {
                text: qsTr("Sort by category")
                icon.name: categorizedIcon(CategorizedSortProxyModel.DDECategory)
                display: D.IconLabel.IconBesideText
                checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.DDECategory
                onTriggered: {
                    CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.DDECategory
                }
            }

            D.MenuItem {
                text: qsTr("Sort by name")
                icon.name: categorizedIcon(CategorizedSortProxyModel.Alphabetary)
                display: D.IconLabel.IconBesideText
                checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary
                onTriggered: {
                    CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.Alphabetary
                }
            }
        }
    }

    D.ToolButton {
        id: title

        implicitHeight: DS.Style.control.implicitHeight(this) * 1.5
        Layout.alignment: Qt.AlignCenter
        KeyNavigation.down: computer
        KeyNavigation.up: setting

        contentItem: Item {
            ColumnLayout {
                anchors.centerIn: parent
                Layout.alignment: Qt.AlignVCenter

                D.DciIcon {
                    width: parent.width
                    height: parent.height / 3 * 2
                    name: categorizedIcon(CategorizedSortProxyModel.categoryType)
                }

                Item {
                    Layout.preferredHeight: 5
                }

                D.DciIcon {
                    name: "back"
                    rotation: 270
                    width: parent.width
                    height: parent.height / 3 * 1
                }
            }
        }

        onClicked: {
            const menu = categorizedCom.createObject(this);
            menu.popup(this, Qt.point(title.x, title.y));
        }
    }

    Item {
        Layout.fillHeight: true
    }

    D.ToolButton {
        id: computer
        icon.name: "computer-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Computer")
        Layout.alignment: Qt.AlignCenter
        focusPolicy: Qt.NoFocus
        KeyNavigation.down: images
        KeyNavigation.up: title
        onClicked: {
            DesktopIntegration.showUrl("computer:///")
        }
    }

    D.ToolButton {
        id: images
        icon.name: "folder-images-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Pictures")
        Layout.alignment: Qt.AlignCenter
        focusPolicy: Qt.NoFocus
        KeyNavigation.down: documents
        KeyNavigation.up: computer
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.PicturesLocation)
        }
    }

    D.ToolButton {
        id: documents
        icon.name: "folder-documents-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Documents")
        Layout.alignment: Qt.AlignCenter
        focusPolicy: Qt.NoFocus
        KeyNavigation.down: desktop
        KeyNavigation.up: images
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DocumentsLocation)
        }
    }

    D.ToolButton {
        id: desktop
        icon.name: "folder-desktop-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Desktop")
        Layout.alignment: Qt.AlignCenter
        focusPolicy: Qt.NoFocus
        KeyNavigation.down: setting
        KeyNavigation.up: documents
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DesktopLocation)
        }
    }

    D.ToolButton {
        id: setting
        icon.name: "setting"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Control Center")
        Layout.alignment: Qt.AlignCenter
        focusPolicy: Qt.NoFocus
        KeyNavigation.down: title
        KeyNavigation.up: desktop
        onClicked: {
            DesktopIntegration.openSystemSettings();
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
