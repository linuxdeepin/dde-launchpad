// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

import "."

ColumnLayout {
    Layout.fillWidth: false
    Layout.preferredWidth: 220
    Layout.fillHeight: true

    DelegateModel {
        id: delegateCategorizedModel
        model: CategorizedSortProxyModel

        delegate: ItemDelegate {
            id: itemDelegate
            text: model.display
            checkable: false
            icon.name: iconName
            width: appListView.width
            font: DTK.fontManager.t8
            // icon.source: "image://app-icon/" + iconName;
            ColorSelector.family: Palette.CrystalColor

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: {
                    showContextMenu(itemDelegate, model, false, false, false)
                }
            }

            Keys.onReturnPressed: {
                launchApp(desktopId)
            }

            TapHandler {
                onTapped: {
                    launchApp(desktopId)
                }
            }

            background: BoxPanel {
                visible: ColorSelector.controlState === DTK.HoveredState
                outsideBorderColor: null
            }
        }
    }

    AppListView {
        id: appListView
        Layout.fillWidth: true
        Layout.fillHeight: true

        model: delegateCategorizedModel
    }
}
