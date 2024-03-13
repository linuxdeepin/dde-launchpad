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
    Layout.fillWidth: false
    Layout.preferredWidth: 220
    Layout.fillHeight: true

    signal freeSortViewFolderClicked(string folderId, string folderName)

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
        sourceComponent: freeSortListView
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Component {
        id: categoryListView
        AppListView {
            id: appCategoryListView
        }
    }

    Component {
        id: freeSortListView
        FreeSortListView {
            id: appFreeSortListView

            onFolderClicked: {
                freeSortViewFolderClicked(folderId, folderName)
            }
        }
    }
}
