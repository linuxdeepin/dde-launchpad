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

            SideBar {}

            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
            }

            AppList {}

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Label {
                    visible: favoriteGridView.visible
                    text: qsTr("My Favorites")
                    font: LauncherController.boldFont(DTK.fontManager.t6)
                }

                Item {
                    id: favoriteGridView
                    visible: searchEdit.text === ""

                    property int rowCount: Math.min(Math.ceil(delegateFavorateModel.count / 4), 2)

                    DelegateModel {
                        id: delegateFavorateModel
                        model: FavoritedProxyModel
                        delegate: IconItemDelegate {
                            iconSource: iconName
                            width: favoriteGridViewContainer.cellSize
                            height: favoriteGridViewContainer.cellSize
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
                        id: favoriteGridViewContainer
                        rows: 0
                        columns: 4
                        placeholderText: qsTr("Add your favorite apps here")
                        model: delegateFavorateModel
                        interactive: favoriteGridView.rowCount > 1
                        activeFocusOnTab: visible && gridViewFocus
                        vScrollBar: ScrollBar {
                        }
                    }

                    Layout.preferredHeight: rowCount === 0 ? 50 : rowCount * favoriteGridViewContainer.cellSize
                    Layout.fillWidth: true
                }

                Label {
                    text: qsTr("All Apps")
                    font: LauncherController.boldFont(DTK.fontManager.t6)
                }

                DelegateModel {
                    id: delegateAllAppsModel
                    model: SearchFilterProxyModel
                    delegate: IconItemDelegate {
                        iconSource: iconName
                        width: allAppsGridContainer.cellSize
                        height: allAppsGridContainer.cellSize
                        onItemClicked: {
                            launchApp(desktopId)
                        }
                        onMenuTriggered: {
                            showContextMenu(this, model, false, false, false)
                        }
                    }
                }

                GridViewContainer {
                    id: allAppsGridContainer
                    rows: 0
                    columns: 4
                    placeholderIcon: "search_no_result"
                    placeholderText: qsTr("No search results")
                    model: delegateAllAppsModel
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    activeFocusOnTab: gridViewFocus
                    vScrollBar: ScrollBar {
                    }

                    MouseArea {
                        enabled: favoriteGridView.visible && (currentIndex !== -1)

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
            }
        }

        Rectangle {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
        }

        RowLayout {
            // Layout.preferredHeight: 20
            Layout.fillWidth: true

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
                        common: ("#transparent")
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
            appListView.positionViewAtBeginning()
            favoriteGridViewContainer.positionViewAtBeginning()
            allAppsGridContainer.positionViewAtBeginning()
        }
    }
}
