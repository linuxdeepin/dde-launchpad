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

Item {
    width: 780
    height: 600
    visible: true

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10

        spacing: 10

        ColumnLayout {
            Layout.fillWidth: false
            Layout.preferredWidth: 300
            Layout.fillHeight: true

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: false

                Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    font: DTK.fontManager.t5
                    text: qsTr("Categories")
                    verticalAlignment: Text.AlignVCenter
                }

                ButtonBox {
                    ColorSelector.family: Palette.CrystalColor
                    ToolButton {
                        icon.name: "title-icon"
                        checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.DDECategory
                        onClicked: {
                            CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.DDECategory
                        }
                    }
                    ToolButton {
                        icon.name: "letter-icon"
                        checked: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary
                        onClicked: {
                            CategorizedSortProxyModel.categoryType = CategorizedSortProxyModel.Alphabetary
                        }
                    }
                }
            }

            DelegateModel {
                id: delegateCategorizedModel
                model: CategorizedSortProxyModel

                delegate: ItemDelegate {
                    id: itemDelegate
                    text: model.display
                    checkable: false
                    icon.name: iconName
        //            icon.source: "image://app-icon/" + iconName;
                    backgroundVisible: false

                    TapHandler {
                        acceptedButtons: Qt.RightButton
                        onTapped: {
                            showContextMenu(itemDelegate, model, false, false, false)
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        onClicked: {
                            launchApp(desktopId)
                        }
                    }
                }
            }

            AppListView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                model: delegateCategorizedModel
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.preferredHeight: 50

//                Rectangle {
//                    color: 'teal'
//                    Layout.fillWidth: true
//                    Layout.fillHeight: true
//                    Text {
//                        anchors.centerIn: parent
//                        text: parent.width + 'x' + parent.height
//                    }
//                }
                Button {
                    text: qsTr("Power")
                    flat: true
                    icon {
                        name: "shutdown"
                        width: 16
                        height: 16
                    }
                    onClicked: {
                        DesktopIntegration.openShutdownScreen();
                    }
                }

                Button {
                    text: qsTr("Settings")
                    flat: true
                    icon {
                        name: "setting"
                        width: 16
                        height: 16
                    }
                    onClicked: {
                        DesktopIntegration.openSystemSettings();
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                Layout.fillHeight: false

                SearchEdit {
                    id: searchEdit
                    Layout.fillWidth: true
                    placeholder: qsTr("Search")
                    onTextChanged: {
                        console.log(text)
                        SearchFilterProxyModel.setFilterRegularExpression(text)
                    }
                }

                IconButton {
                    icon.name: "switch_to_fullscreen"
                    ColorSelector.family: Palette.CrystalColor
                    onClicked: {
                        LauncherController.currentFrame = "FullscreenFrame"
                    }
                }
            }

            Label {
                visible: favoriteGridView.visible
                text: qsTr("My Favorites")
            }

            Rectangle {
                id: favoriteGridView
                visible: searchEdit.text === ""
                color: "transparent"

                property int rowCount: Math.min(Math.ceil(delegateFavorateModel.count / 4), 2)

                DelegateModel {
                    id: delegateFavorateModel
                    model: FavoritedProxyModel
                    delegate: IconItemDelegate {
                        iconSource: "image://app-icon/" + iconName
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
                }

                Layout.preferredHeight: rowCount === 0 ? 50 : rowCount * favoriteGridViewContainer.cellSize
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("All Apps")
            }

            GridViewContainer {
                id: allAppsGridContainer
                rows: 0
                columns: 4
                model: SearchFilterProxyModel
                delegate: IconItemDelegate {
                    iconSource: "image://app-icon/" + iconName
                    width: allAppsGridContainer.cellSize
                    height: allAppsGridContainer.cellSize
                    onItemClicked: {
                        launchApp(desktopId)
                    }
                    onMenuTriggered: {
                        showContextMenu(this, model, false, false, false)
                    }
                }
                Layout.fillWidth: true
                Layout.fillHeight: true

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
}
