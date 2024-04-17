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

    KeyNavigation.tab: appGridLoader.item

    MouseArea {
        anchors.fill: parent
        onClicked: () => {
            baseLayer.focus = true
        }
    }

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

    SideBar {
        id: sideBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 5
        nextKeyTabTarget: bottomBar.keyTabTarget
    }

    Control {
        id: rowLineControl
        width: Helper.windowed.splitLineWidth
        anchors.left: sideBar.right
        anchors.top: baseLayer.top
        anchors.bottom: bottomBar.top
        anchors.leftMargin: 5

        property Palette backgroundColor: Palette {
            normal {
                common: Qt.rgba(0, 0, 0, 0.05)
                crystal: Qt.rgba(0, 0, 0, 0.05)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.05)
                crystal: Qt.rgba(1, 1, 1, 0.05)
            }
        }

        contentItem: Rectangle {
            color: rowLineControl.ColorSelector.backgroundColor
        }
    }

    RowLayout {
        id: appArea
        anchors.left: sideBar.right
        anchors.top: sideBar.top
        anchors.right: parent.right
        anchors.bottom: bottomBar.top
        anchors.topMargin: 10
        anchors.leftMargin: 5
        spacing: 0
        AppList {
            id: appList
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            nextKeyTabTarget: sideBar.keyTabTarget
        }

        Loader {
            id: appGridLoader
            property Item keyTabTarget: appGridLoader.item.keyTabTarget
            Component {
                id: analysisViewCom
                AnalysisView {
                    nextKeyTabTarget: appList.keyTabTarget
                }
            }
            Component {
                id: searchResultViewCom
                SearchResultView {
                    nextKeyTabTarget: appList.keyTabTarget
                }
            }
            Layout.fillHeight: true
            Layout.preferredWidth: 362
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.leftMargin: Helper.frequentlyUsed.leftMargin
            Layout.rightMargin: Helper.frequentlyUsed.rightMargin
            sourceComponent: bottomBar.searchEdit.text === "" ? analysisViewCom
                : searchResultViewCom
        }
    }

    BottomBar {
        id: bottomBar
        height: 40
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        nextKeyTabTarget: appGridLoader.keyTabTarget
    }

    Control {
        id: columnLineControl
        height: Helper.windowed.splitLineWidth
        anchors.left: baseLayer.left
        anchors.right: baseLayer.right
        anchors.bottom: bottomBar.top

        property Palette backgroundColor: Palette {
            normal {
                common: Qt.rgba(0, 0, 0, 0.05)
                crystal: Qt.rgba(0, 0, 0, 0.05)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.05)
                crystal: Qt.rgba(1, 1, 1, 0.05)
            }
        }

        contentItem: Rectangle {
            color: columnLineControl.ColorSelector.backgroundColor
        }
    }

    FolderGridViewPopup {
        id: folderGridViewPopup
        width: 370
        height: 312
        folderNameFont: LauncherController.adjustFontWeight(DTK.fontManager.t6, Font.Bold)

        onVisibleChanged: function (visible) {
            if (!visible) {
                appArea.opacity = 1
            }
        }
    }

    Keys.onPressed: function (event) {
        if (bottomBar.searchEdit.focus === false && !bottomBar.searchEdit.text && (event.text && !"\t\r\0 ".includes(event.text))) {
            bottomBar.searchEdit.focus = true
            bottomBar.searchEdit.text = event.text
        } else if (bottomBar.searchEdit.focus === true || baseLayer.focus === true) {
            // the SearchEdit will catch the key event first, and events that it won't accept will then got here
            switch (event.key) {
            case Qt.Key_Up:
            case Qt.Key_Down:
                appGridLoader.item.forceActiveFocus()
                return;
            case Qt.Key_Enter:
            case Qt.Key_Return:
                if (bottomBar.searchEdit.text !== "") {
                    appGridLoader.item.launchCurrentItem()
                } else {
                    appGridLoader.item.forceActiveFocus()
                }
            }
        }
    }

    Keys.onEscapePressed: function (event) {
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

            appArea.opacity = 0.1
        }
    }
}
