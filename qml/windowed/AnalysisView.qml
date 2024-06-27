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

    property Item keyTabTarget: frequentlyUsedView.keyTabTarget
    property Item nextKeyTabTarget
    focus: true

    onFocusChanged: () => {
        frequentlyUsedView.focus = true
    }

    function positionViewAtBeginning() {
        frequentlyUsedView.positionViewAtBeginning()
        recentlyInstalledView.positionViewAtBeginning()
    }

    // Binding SearchFilterProxyModel with RecentlyInstalledProxyModel
    Binding {
        target: FrequentlyUsedProxyModel; property: "recentlyInstalledModel"
        value: recentlyInstalledView.model
    }

    contentItem: ColumnLayout {
        spacing: 0
        focus: true

        FrequentlyUsedView {
            id: frequentlyUsedView
            visible: count > 0
            focus: visible
            maxCount: recentlyInstalledView.visible ? 12 : 16
            nextKeyTabTarget : recentlyInstalledView.visible ? recentlyInstalledView.keyTabTarget : control.nextKeyTabTarget
        }

        RecentlyInstalledView {
            id: recentlyInstalledView
            visible: count > 0
            Layout.topMargin: -(Helper.frequentlyUsed.cellPaddingRows / 2)
            nextKeyTabTarget: control.nextKeyTabTarget
        }

        Item {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
        }
    }

    background: DebugBounding { }
}
