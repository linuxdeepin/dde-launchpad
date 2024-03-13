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

import "windowed"

Item {
    id: baseLayer
    objectName: "WindowedFrame-BaseLayer"

    visible: true
    focus: true

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Helper.windowed.topMargin

            SideBar {}

            Rectangle {
                Layout.preferredWidth: 2
                Layout.fillHeight: true
                color: this.palette.shadow
            }

            AppList {
                id: appList
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.rightMargin: Helper.frequentlyUsed.rightMargin
                Layout.margins: 0

                // Binding SearchFilterProxyModel with RecentlyInstalledProxyModel
                Binding {
                    target: SearchFilterProxyModel; property: "recentlyInstalledModel"
                    value: delegateRecentlyInstalledModel.model
                }

                Label {
                    text: qsTr("My FrequentlyUseds")
                    font: LauncherController.boldFont(DTK.fontManager.t6)
                }

                DelegateModel {
                    id: delegateFavorateModel
                    model: CountLimitProxyModel {
                        sourceModel: SearchFilterProxyModel
                        maxRowCount: recentlyInstalledView.visible ? 12 : 16
                    }
                    delegate: IconItemDelegate {
                        iconSource: iconName
                        width: frequentlyUsedViewContainer.cellWidth
                        height: frequentlyUsedViewContainer.cellHeight
                        onItemClicked: {
                            launchApp(desktopId)
                        }
                        onMenuTriggered: {
                            showContextMenu(this, model, false, false, false)
                        }
                    }
                }

                GridViewContainer {
                    id: frequentlyUsedViewContainer
                    rows: 0
                    columns: 4
                    placeholderIcon: "search_no_result"
                    placeholderText: qsTr("No search results")
                    model: delegateFavorateModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    activeFocusOnTab: gridViewFocus
                    interactive: false

                    MouseArea {
                        enabled: recentlyInstalledView.visible && (currentIndex !== -1)

                        property int currentIndex: -1
                        property int modelType: -1 // 1: fav, 2: all

                        anchors.fill: parent
                        onPressAndHold: {
                            if (index !== -1) {
                                currentIndex = index
                            }
                        }
                        onReleased: currentIndex = -1
                    }
                }

                Label {
                    visible: recentlyInstalledView.visible
                    text: qsTr("Recently Installed")
                    font: LauncherController.boldFont(DTK.fontManager.t6)
                }

                Item {
                    id: recentlyInstalledView
                    visible: searchEdit.text === "" && delegateRecentlyInstalledModel.count > 0

                    DelegateModel {
                        id: delegateRecentlyInstalledModel
                        model: CountLimitProxyModel {
                            sourceModel: RecentlyInstalledProxyModel
                            maxRowCount: 4
                        }

                        delegate: IconItemDelegate {
                            iconSource: iconName
                            width: recentlyInstalledViewContainer.cellWidth
                            height: recentlyInstalledViewContainer.cellHeight
                            onItemClicked: {
                                launchApp(desktopId)
                            }
                            onMenuTriggered: {
                                showContextMenu(this, model, false, true, false)
                            }
                        }
                    }

                    GridViewContainer {
                        anchors.fill: parent
                        id: recentlyInstalledViewContainer
                        rows: 0
                        columns: 4
                        model: delegateRecentlyInstalledModel
                        interactive: false
                        activeFocusOnTab: visible && gridViewFocus
                    }

                    Layout.preferredHeight: recentlyInstalledViewContainer.cellHeight
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            Layout.preferredHeight: 2
            Layout.fillWidth: true
            color: this.palette.shadow
        }

        RowLayout {
            // Layout.preferredHeight: 20
            Layout.fillWidth: true
            Layout.margins: 10

            ToolButton {
                icon.name: "shutdown"
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                ToolTip.text: qsTr("Power")
                onClicked: {
                    DesktopIntegration.openShutdownScreen();
                }
            }

            SearchEdit {
                id: searchEdit

                Layout.leftMargin: parent.width / 4
                Layout.rightMargin: parent.width / 4

                Layout.fillWidth: true
                placeholder: qsTr("Search")
                onTextChanged: {
                    console.log(text)
                    SearchFilterProxyModel.setFilterRegularExpression(text)
                    SearchFilterProxyModel.invalidate()
                }

                Palette {
                    id: actionPalette
                    normal {
                        common: ("#f7f7f7")
                        crystal: Qt.rgba(0, 0, 0, 0.1)
                    }
                    normalDark {
                        common: Qt.rgba(1, 1, 1, 0.1)
                        crystal: Qt.rgba(1, 1, 1, 0.1)
                    }
                    hovered {
                        common: ("#e1e1e1")
                        crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
                    }
                    pressed {
                        common: ("#bcc4d0")
                        crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
                    }
                }

                Palette {
                    id: nomalPalette
                    normal {
                        common: ("transparent")
                        crystal: Qt.rgba(0, 0, 0, 0.1)
                    }
                    normalDark {
                        common: ("transparent")
                        crystal: Qt.rgba(1, 1, 1, 0.1)
                    }
                    hovered {
                        common: ("#e1e1e1")
                        crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
                    }
                    pressed {
                        common: ("#bcc4d0")
                        crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
                    }
                }

                backgroundColor: searchEdit.editting ? actionPalette : nomalPalette
            }

            ToolButton {
                icon.name: "launcher_fullscreen"
                Accessible.name: "Fullscreen"
                onClicked: {
                    LauncherController.currentFrame = "FullscreenFrame"
                }
            }
        }
    }

    Keys.onPressed: {
        if (searchEdit.focus === false && !searchEdit.text && (event.text && !"\t ".includes(event.text))) {
            searchEdit.focus = true
            searchEdit.text = event.text
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
            searchEdit.text = ""
            // reset(remove) keyboard focus
            baseLayer.focus = true
            // reset scroll area position
            appList.positionViewAtBeginning()
            frequentlyUsedViewContainer.positionViewAtBeginning()
            recentlyInstalledViewContainer.positionViewAtBeginning()
        }
    }
}
