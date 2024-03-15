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

    property string searchingText

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
            maxCount: recentlyInstalledView.visible ? 12 : 16
        }

        RecentlyInstalledView {
            id: recentlyInstalledView
            visible: searchingText === "" && count > 0
            Layout.topMargin: -(Helper.frequentlyUsed.cellPaddingRows / 2)
        }

        Item {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        visible: searchingText && frequentlyUsedView.count <= 0
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
