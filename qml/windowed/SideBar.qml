// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.platform 1.0
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

ColumnLayout {
    Layout.fillWidth: false
    Layout.fillHeight: true

    ToolButton {
        icon.name: "title-icon"
        checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.DDECategory
        onClicked: {
            CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.DDECategory
        }
    }

    ToolButton {
        icon.name: "letter-icon"
        checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary
        onClicked: {
            CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.Alphabetary
        }
    }

    Item {
        Layout.fillHeight: true
    }

    ToolButton {
        icon.name: "computer-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Pictures")
        onClicked: {
            DesktopIntegration.showUrl("computer:///")
        }
    }

    ToolButton {
        icon.name: "folder-images-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Pictures")
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.PicturesLocation)
        }
    }

    ToolButton {
        icon.name: "folder-documents-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Documents")
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DocumentsLocation)
        }
    }

    ToolButton {
        icon.name: "folder-desktop-symbolic"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Desktop")
        onClicked: {
            DesktopIntegration.showFolder(StandardPaths.DesktopLocation)
        }
    }

    ToolButton {
        icon.name: "setting"
        ToolTip.visible: hovered
        ToolTip.delay: 1000
        ToolTip.text: qsTr("Control Center")
        onClicked: {
            DesktopIntegration.openSystemSettings();
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
