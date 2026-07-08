// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

import "."

ColumnLayout {

    signal freeSortViewFolderClicked(string folderId, string folderName, point triggerPosition)

    property bool isFreeSort: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.FreeCategory
    property Item keyTabTarget: isFreeSort ? freeSortView.keyTabTarget : categoryView.keyTabTarget
    property Item nextKeyTabTarget

    onFocusChanged: () => {
        (isFreeSort ? freeSortView : categoryView).focus = true
    }

    onIsFreeSortChanged: {
        (isFreeSort ? freeSortView : categoryView).resetViewState()
    }

    function resetViewState() {
        (isFreeSort ? freeSortView : categoryView).resetViewState()
    }

    // Both views are always instantiated; switching is done via `visible`
    // to avoid the costly Loader component destruction/creation (~120ms -> ~0ms).
    // The hidden view's ListView retains its delegates because it keeps a valid
    // size from anchors.fill, so re-showing is instant.
    // Extra memory: ~60-80KB for hidden view delegates (measured).
    Item {
        id: viewContainer
        Layout.fillWidth: true
        Layout.fillHeight: true

        AppListView {
            id: categoryView
            anchors.fill: viewContainer
            visible: !isFreeSort

            MouseAreaCom {}
            KeyNavigation.tab: nextKeyTabTarget
        }

        FreeSortListView {
            id: freeSortView
            anchors.fill: viewContainer
            visible: isFreeSort

            onFolderClicked: {
                freeSortViewFolderClicked(folderId, folderName, triggerPosition)
            }

            MouseAreaCom {}
            KeyNavigation.tab: nextKeyTabTarget
        }
    }

    component MouseAreaCom: MouseArea {
        anchors.fill: parent

        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton

        onWheel: function (wheel) {
            closeContextMenu()
            wheel.accepted = false
        }
    }
}
