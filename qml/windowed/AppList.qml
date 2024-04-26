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
import org.deepin.launchpad.models 1.0

import "."

ColumnLayout {

    signal freeSortViewFolderClicked(string folderId, string folderName)

    property Item keyTabTarget: loader.item
    property Item nextKeyTabTarget

    onFocusChanged: () => {
        loader.item.focus = true
    }

    function positionViewAtBeginning() {
        loader.item.positionViewAtBeginning()
    }

    function switchToFreeSort(freeSort) {
        if (freeSort) {
            loader.sourceComponent = freeSortListView
        } else {
            loader.sourceComponent = categoryListView
        }
    }

    Loader {
        id: loader
        sourceComponent: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.FreeCategory ? freeSortListView : categoryListView
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Component {
        id: categoryListView
        AppListView {
            id: appCategoryListView

            MouseAreaCom {}
            nextKeyTabTargetItem: nextKeyTabTarget
        }
    }

    Component {
        id: freeSortListView
        FreeSortListView {
            id: appFreeSortListView

            onFolderClicked: {
                freeSortViewFolderClicked(folderId, folderName)
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
