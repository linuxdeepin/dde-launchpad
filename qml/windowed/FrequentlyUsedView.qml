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

    onFocusChanged: () => {
        frequentlyUsedViewContainer.focus = true
    }

    property Item nextKeyTabTarget
    property Item keyTabTarget: frequentlyUsedViewContainer

    property int count: frequentlyUsedViewContainer.count
    property int maxCount: 16

    function positionViewAtBeginning() {
        frequentlyUsedViewContainer.positionViewAtBeginning()
    }

    contentItem: ColumnLayout {
        spacing: 0

        Label {
            text: qsTr("Frequently Used")
            font: LauncherController.adjustFontWeight(DTK.fontManager.t6, Font.Bold)
        }

        GridViewContainer {
            id: frequentlyUsedViewContainer

            KeyNavigation.tab: control.nextKeyTabTarget
            Layout.alignment: Qt.AlignRight
            Layout.topMargin: 10
            Layout.preferredHeight: frequentlyUsedViewContainer.height
            Layout.preferredWidth: frequentlyUsedViewContainer.width
            interactive: false

            model: CountLimitProxyModel {
                sourceModel: FrequentlyUsedProxyModel
                maxRowCount: maxCount
            }

            delegate: IconItemDelegate {
                width: frequentlyUsedViewContainer.cellWidth
                height: frequentlyUsedViewContainer.cellHeight
                iconSource: iconName
                onItemClicked: {
                    launchApp(desktopId)
                }
                onMenuTriggered: {
                    showContextMenu(this, model)
                }
            }

            activeFocusOnTab: gridViewFocus
        }
    }

    background: DebugBounding { }
}
