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

StackView {
    id: stackView
    width: 780
    height: 600
    visible: true
    focus: true

    initialItem: Item {
        id: baseLayer
        objectName: "WindowedFrame-BaseLayer"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10

            spacing: 10

            ColumnLayout {
                Layout.fillWidth: false
                Layout.preferredWidth: 220
                Layout.fillHeight: true

                DelegateModel {
                    id: delegateCategorizedModel
                    model: CategorizedSortProxyModel

                    delegate: ItemDelegate {
                        id: itemDelegate
                        text: model.display
                        checkable: false
                        icon.name: iconName
                        width: appListView.width
            //            icon.source: "image://app-icon/" + iconName;
                        ColorSelector.family: Palette.CrystalColor

                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: {
                                showContextMenu(itemDelegate, model, false, false, false)
                            }
                        }

                        Keys.onReturnPressed: {
                            launchApp(desktopId)
                        }

                        TapHandler {
                            onTapped: {
                                launchApp(desktopId)
                            }
                        }

                        background: BoxPanel {
                            visible: ColorSelector.controlState === DTK.HoveredState
                            outsideBorderColor: null
                        }
                    }
                }

                AppListView {
                    id: appListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    model: delegateCategorizedModel

                    onSectionHeaderClicked: {
                        if (categoryType === CategorizedSortProxyModel.Alphabetary) {
                            stackView.push(catalogy.createObject(null, {
                                'existingSections': CategorizedSortProxyModel.alphabetarySections()
                            }))
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    Layout.preferredHeight: 30
                    Layout.topMargin: 5
                    spacing: 10

                    ToolButton {
                        icon.name: "shutdown"
                        ToolTip.visible: hovered
                        ToolTip.delay: 1000
                        ToolTip.text: qsTr("Power")
                        onClicked: {
                            DesktopIntegration.openShutdownScreen();
                        }
                    }

                    ToolButton {
                        icon.name: "setting"
                        ToolTip.visible: hovered
                        ToolTip.delay: 1000
                        ToolTip.text: qsTr("Control Center")
                        onClicked: {
                            DesktopIntegration.openSystemSettings();
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    ButtonBox {
                        Layout.preferredHeight: 30
                        padding: 0
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
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Label {
                    visible: favoriteGridView.visible
                    text: qsTr("My Favorites")
                    font {
                        pixelSize: DTK.fontManager.fontPixelSize(FontManager.T6)
                        bold: true
                    }
                }

                Item {
                    id: favoriteGridView
                    visible: searchEdit.text === ""

                    property int rowCount: Math.min(Math.ceil(delegateFavorateModel.count / 4), 2)

                    DelegateModel {
                        id: delegateFavorateModel
                        model: FavoritedProxyModel
                        delegate: IconItemDelegate {
                            iconSource: "image://app-icon/" + DTK.platformTheme.iconThemeName + "/" + iconName
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
                        vScrollBar: ScrollBar {}
                    }

                    Layout.preferredHeight: rowCount === 0 ? 50 : rowCount * favoriteGridViewContainer.cellSize
                    Layout.fillWidth: true
                }

                Label {
                    text: qsTr("All Apps")
                    font {
                        pixelSize: DTK.fontManager.fontPixelSize(FontManager.T6)
                        bold: true
                    }
                }

                DelegateModel {
                    id: delegateAllAppsModel
                    model: SearchFilterProxyModel
                    delegate: IconItemDelegate {
                        iconSource: "image://app-icon/" + DTK.platformTheme.iconThemeName + "/" + iconName
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
                    vScrollBar: ScrollBar {}

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

                // Workaround: cannot use RowLayout directly due to unknown layout bug from Qt, so we wrapped with a Control here.
                Control {
                    Layout.fillWidth: true
                    Layout.topMargin: 5

                    contentItem: RowLayout {
                        Layout.fillHeight: false

                        SearchEdit {
                            id: searchEdit

                            Layout.leftMargin: width / 5
                            Layout.rightMargin: width / 5

                            Layout.fillWidth: true
                            placeholder: qsTr("Search")
                            onTextChanged: {
                                console.log(text)
                                SearchFilterProxyModel.setFilterRegularExpression(text)
                            }
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
            }
        }
    }

    Component {
        id: catalogy

        Item {
            id: catalogyItem

            property var existingSections: []

            AlphabetCategory {
                id: alphabetCatalogy
                existingSections: catalogyItem.existingSections

                anchors.fill: parent
                anchors.margins: 10

                onBackClicked: {
                    stackView.pop()
                }

                onCategoryClicked: {
                    appListView.scrollToAlphabetCategory(character)
                    stackView.pop()
                }
            }
        }
    }
    Keys.onPressed: {
        if ((event.text && !"\t ".includes(event.text)) && (stackView.currentItem !== stackView.initialItem)) {
            stackView.pop()
        }
        if (searchEdit.focus === false && !searchEdit.text && event.text) {
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
            // exit from AlphabetCatalogy screen when
            if (stackView.currentItem !== stackView.initialItem) {
                stackView.pop()
            }
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
