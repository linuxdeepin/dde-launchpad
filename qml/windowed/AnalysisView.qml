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

    property bool searchingMode

    function positionViewAtBeginning() {
        frequentlyUsedView.positionViewAtBeginning()
        recentlyInstalledView.positionViewAtBeginning()
    }

    // Binding SearchFilterProxyModel with RecentlyInstalledProxyModel
    Binding {
        target: SearchFilterProxyModel; property: "recentlyInstalledModel"
        value: recentlyInstalledView.model
    }

    contentItem: ColumnLayout {
        spacing: 0

        FrequentlyUsedView {
            id: frequentlyUsedView
            visible: count > 0
            maxCount: recentlyInstalledView.visible ? 12 : !searchingMode ? 16 : -1
            searchingMode: control.searchingMode
        }

        RecentlyInstalledView {
            id: recentlyInstalledView
            visible: !searchingMode && count > 0
            Layout.topMargin: -(Helper.frequentlyUsed.cellPaddingRows / 2)
        }

        Item {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        visible: searchingMode && frequentlyUsedView.count <= 0
        DciIcon {
            Layout.alignment: Qt.AlignCenter
            sourceSize {
                width: 128
                height: width
            }
            name: "search_no_result"
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("No search results")
        }
    }

    background: DebugBounding { }
}
