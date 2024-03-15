// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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

Item {
    id: baseLayer
    objectName: "WindowedFrame-BaseLayer"

    visible: true
    focus: true

    // ----------- Drag and Drop related functions START -----------
    Label {
        property string currentlyDraggedId

        id: dndItem
        visible: DebugHelper.qtDebugEnabled
        text: "DnD DEBUG"

        Drag.onActiveChanged: {
            if (Drag.active) {
                text = "Dragging " + currentlyDraggedId
            } else {
                currentlyDraggedId = ""
            }
        }
    }

    function dropOnItem(dragId, dropId, op) {
        dndItem.text = "drag " + dragId + " onto " + dropId + " with " + op
        ItemArrangementProxyModel.commitDndOperation(dragId, dropId, op)
    }

    function dropOnPage(dragId, dropFolderId, pageNumber) {
        dndItem.text = "drag " + dragId + " into " + dropFolderId + " at page " + pageNumber
        ItemArrangementProxyModel.commitDndOperation(dragId, dropFolderId, ItemArrangementProxyModel.DndJoin, pageNumber)
    }
    // ----------- Drag and Drop related functions  END  -----------

    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        Layout.margins: 0

        RowLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Helper.windowed.topMargin

            SideBar {
                id: sideBar
            }

            Rectangle {
                Layout.preferredWidth: 2
                Layout.fillHeight: true
                color: this.palette.shadow
            }

            AppList {
                id: appList
                Layout.fillWidth: true
            }

            AnalysisView {
                id: analysisView

                Layout.fillHeight: true
                Layout.preferredWidth: 362
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                Layout.rightMargin: Helper.frequentlyUsed.rightMargin
                searchingText: bottomBar.searchEdit.text
            }
        }

        Rectangle {
            Layout.preferredHeight: 2
            Layout.fillWidth: true
            color: this.palette.shadow
        }

        BottomBar {
            id: bottomBar
            // Layout.preferredHeight: 20
            Layout.fillWidth: true
            Layout.margins: 10
        }
    }

    FolderGridViewPopup {
        id: folderGridViewPopup
        backgroundAlph: 0.8

        onVisibleChanged: {
            if (!visible) {
                baseLayer.opacity = 1
            }
        }
    }

    Keys.onPressed: {
        if (bottomBar.searchEdit.focus === false && !bottomBar.searchEdit.text && (event.text && !"\t ".includes(event.text))) {
            bottomBar.searchEdit.focus = true
            bottomBar.searchEdit.text = event.text
        }
    }

    Keys.onEscapePressed: {
        if (!DebugHelper.avoidHideWindow) {
            LauncherController.visible = false;
        }
    }

    Connections {
        target: LauncherController
        function onVisibleChanged() {
            // only do these clean-up steps on launcher get hide
            if (LauncherController.visible) return

            // clear searchEdit text
            bottomBar.searchEdit.text = ""
            // reset(remove) keyboard focus
            baseLayer.focus = true
            // reset scroll area position
            appList.positionViewAtBeginning()
        }
    }

    Connections {
        target: sideBar
        function onSwitchToFreeSort(isFreeSort) {
            appList.switchToFreeSort(isFreeSort)
        }
    }

    Connections {
        target: appList
        function onFreeSortViewFolderClicked(folderId, folderName) {
            folderGridViewPopup.currentFolderId = folderId
            folderGridViewPopup.folderName = folderName
            folderGridViewPopup.open()

            baseLayer.opacity = 0.1
        }
    }
}
