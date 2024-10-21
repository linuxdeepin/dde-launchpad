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

    property Item nextKeyTabTarget
    property Item keyTabTarget: recentlyInstalledViewContainer

    readonly property int count: recentlyInstalledViewContainer.count
    property alias model: limitCountModel.sourceModel

    onFocusChanged: () => {
        recentlyInstalledViewContainer.focus = true
    }

    function positionViewAtBeginning() {
        recentlyInstalledViewContainer.positionViewAtBeginning()
    }

    contentItem: ColumnLayout {
        spacing: 0
        Label {
            text: qsTr("Recently Installed")
            font: LauncherController.adjustFontWeight(DTK.fontManager.t6, Font.Bold)
        }

        GridViewContainer {
            id: recentlyInstalledViewContainer

            KeyNavigation.tab: nextKeyTabTarget
            Layout.alignment: Qt.AlignRight
            Layout.preferredHeight: recentlyInstalledViewContainer.height
            Layout.preferredWidth: recentlyInstalledViewContainer.width
            Layout.topMargin: 6

            model: CountLimitProxyModel {
                id: limitCountModel
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
                    showContextMenu(this, model)
                    baseLayer.focus = true
                }
            }
            activeFocusOnTab: visible && gridViewFocus
        }
    }

    background: DebugBounding { }
}
