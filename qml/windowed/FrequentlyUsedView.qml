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

    // ??
    onFocusChanged: (focus) => {
        if (focus)
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
        focus: true

        Label {
            text: qsTr("Frequently Used")
            font: LauncherController.adjustFontWeight(DTK.fontManager.t6, Font.Bold)
        }

        GridViewContainer {
            id: frequentlyUsedViewContainer
            focus: true

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
                focus: true
                width: frequentlyUsedViewContainer.cellWidth
                height: frequentlyUsedViewContainer.cellHeight
                iconSource: iconName
                onItemClicked: {
                    launchApp(desktopId)
                }
                onMenuTriggered: {
                    showContextMenu(this, model, false, false, true)
                }
            }

            activeFocusOnTab: gridViewFocus
        }
    }

    background: DebugBounding { }
}
