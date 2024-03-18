// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import "."

Control {
    id: control

    readonly property int count: recentlyInstalledViewContainer.count
    readonly property var model: recentlyInstalledViewContainer.model

    function positionViewAtBeginning() {
        recentlyInstalledViewContainer.positionViewAtBeginning()
    }

    contentItem: ColumnLayout {
        spacing: 0
        Label {
            text: qsTr("Recently Installed")
            font: LauncherController.boldFont(DTK.fontManager.t6)
        }

        GridViewContainer {
            id: recentlyInstalledViewContainer

            Layout.alignment: Qt.AlignRight
            Layout.preferredHeight: recentlyInstalledViewContainer.height
            Layout.preferredWidth: recentlyInstalledViewContainer.width
            Layout.topMargin: 6

            model: CountLimitProxyModel {
                // TODO removing sourceModel's binding
                property var holder: RecentlyInstalledProxyModel
                sourceModel: holder
                maxRowCount: 4
            }

            delegate: IconItemDelegate {
                iconSource: iconName
                width: recentlyInstalledViewContainer.cellWidth
                height: recentlyInstalledViewContainer.cellHeight
                onItemClicked: {
                    launchApp(desktopId)
                }
                onMenuTriggered: {
                    showContextMenu(this, model, false, true, false)
                }
            }
            activeFocusOnTab: visible && gridViewFocus
        }
    }

    background: DebugBounding { }
}
