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

    onFocusChanged: () => {
        frequentlyUsedView.focus = true
    }

    function positionViewAtBeginning() {
        frequentlyUsedView.positionViewAtBeginning()
        recentlyInstalledView.positionViewAtBeginning()
    }

    contentItem: ColumnLayout {
        spacing: 0

        FrequentlyUsedProxyModel {
            id: freqUsedModel
            sourceModel: AppsModel
            desktopIdRole: AppItem.DesktopIdRole
            launchedTimesRole: AppItem.LaunchedTimesRole
            lastLaunchedTimeRole: AppItem.LastLaunchedTimeRole
        }

        FrequentlyUsedView {
            id: frequentlyUsedView
            model: freqUsedModel
            visible: count > 0
            maxCount: recentlyInstalledView.visible ? 12 : 16
            nextKeyTabTarget : recentlyInstalledView.visible ? recentlyInstalledView.keyTabTarget : control.nextKeyTabTarget
        }

        RecentlyInstalledView {
            id: recentlyInstalledView
            model: RecentlyInstalledProxyModel {
                sourceModel: AppsModel
                installedTimeRole: AppItem.InstalledTimeRole
                lastLaunchedTimeRole: AppItem.LastLaunchedTimeRole
            }
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
