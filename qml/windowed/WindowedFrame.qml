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

InputEventItem {
    id: baseLayer
    objectName: "WindowedFrame-BaseLayer"

    visible: true
    focus: true

    KeyNavigation.tab: appGridLoader.item

    Shortcut {
        context: Qt.ApplicationShortcut
        sequences: [StandardKey.HelpContents, "F1"]
        onActivated: LauncherController.showHelp()
        onActivatedAmbiguously: LauncherController.showHelp()
    }

    function getHorizontalCoordinatesOfSideBar()
    {
        return sideBar.x + sideBar.width / 2
    }

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
        anchors.topMargin: 10
        anchors.leftMargin: 10
        nextKeyTabTarget: bottomBar.keyTabTarget
    }

    Control {
        id: rowLineControl
        width: Helper.windowed.splitLineWidth
        anchors.left: sideBar.right
        anchors.top: baseLayer.top
        anchors.bottom: bottomBar.top
        anchors.leftMargin: 10

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
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: bottomBar.top
        anchors.topMargin: 10
        anchors.leftMargin: 10
        spacing: 0
        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
        }
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
            Layout.preferredWidth: 365
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            Layout.leftMargin: Helper.frequentlyUsed.leftMargin
            Layout.rightMargin: Helper.frequentlyUsed.rightMargin
            sourceComponent: bottomBar.searchEdit.text === "" ? analysisViewCom
                : searchResultViewCom
        }
    }

    BottomBar {
        id: bottomBar
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
        height: 330
        folderNameFont: LauncherController.adjustFontWeight(DTK.fontManager.t6, Font.BoldFont.Bold)
        centerPosition: Qt.point(curPointX, curPointY)

        readonly property int animationDuration: 200
        property int startPointX: 0
        property int startPointY: 0
        readonly property point endPoint: Qt.point(parent.width / 2, parent.height / 2)
        property int curPointX: 0
        property int curPointY: 0

        onVisibleChanged: function (visible) {
            if (!visible) {
                appArea.opacity = 1
            }
        }

        enter: Transition {
            ParallelAnimation {
                NumberAnimation {
                    duration: folderGridViewPopup.animationDuration
                    properties: "scale"
                    easing.type: Easing.OutQuad
                    from: 0.1
                    to: 1
                }
                NumberAnimation {
                    duration: folderGridViewPopup.animationDuration
                    properties: "curPointX"
                    easing.type: Easing.OutQuad
                    from: folderGridViewPopup.startPointX
                    to: folderGridViewPopup.endPoint.x
                }
                NumberAnimation {
                    duration: folderGridViewPopup.animationDuration
                    properties: "curPointY"
                    easing.type: Easing.OutQuad
                    from: folderGridViewPopup.startPointY
                    to: folderGridViewPopup.endPoint.y
                }
            }
        }

        exit: Transition {
            ParallelAnimation {
                NumberAnimation {
                    duration: folderGridViewPopup.animationDuration
                    properties: "scale"
                    easing.type: Easing.InQuad
                    from: 1
                    to: 0.1
                }
                NumberAnimation {
                    duration: folderGridViewPopup.animationDuration
                    properties: "curPointX"
                    easing.type: Easing.InQuad
                    to: folderGridViewPopup.startPointX
                    from: folderGridViewPopup.endPoint.x
                }
                NumberAnimation {
                    duration: folderGridViewPopup.animationDuration
                    properties: "curPointY"
                    easing.type: Easing.InQuad
                    to: folderGridViewPopup.startPointY
                    from: folderGridViewPopup.endPoint.y
                }
            }
        }
    }

    Keys.forwardTo: [bottomBar.searchEdit]
    Keys.onPressed: function (event) {
        if (bottomBar.searchEdit.focus === true || baseLayer.focus === true) {
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
    onInputReceived: function(text){
        if (bottomBar.searchEdit.text !== "" || bottomBar.searchEdit.focus !== true) {
            bottomBar.searchEdit.text = text
            bottomBar.searchEdit.focus = true
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
            // reset scroll area position and state
            appList.resetViewState()
            folderGridViewPopup.close()
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
        function onFreeSortViewFolderClicked(folderId, folderName, triggerPosition) {
            let point = mapFromItem(appList, triggerPosition)
            folderGridViewPopup.startPointX = point.x
            folderGridViewPopup.startPointY = point.y
            folderGridViewPopup.currentFolderId = folderId
            folderGridViewPopup.folderName = folderName
            folderGridViewPopup.open()

            appArea.opacity = 0.1
        }
    }
}
