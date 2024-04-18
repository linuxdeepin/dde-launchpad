// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15 as QQC2
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Item {
    id: root

    property alias model: listView.model

    function positionViewAtBeginning() {
        listView.positionViewAtBeginning()
    }

    function scrollToAlphabetCategory(character) {
        for (let i = 0; i < model.count; i++) {
            let transliterated1st = model.model.data(model.modelIndex(i), AppsModel.TransliteratedRole)[0].toUpperCase()
            if (character === transliterated1st) {
                listView.currentIndex = i
                scrollToIndex(listView.currentIndex, 35) // the height of a section heading
                break
            }
        }
    }

    function scrollToDDECategory(category) {
        for (let i = 0; i < model.count; i++) {
            let value = model.model.data(model.modelIndex(i), AppItem.DDECategoryRole)
            if (category === value) {
                listView.currentIndex = i
                scrollToIndex(listView.currentIndex, 35) // the height of a section heading
                break
            }
        }
    }

    function scrollToIndex(index, expectedHighlightBegin) {
        let snapMode = listView.snapMode
        let preferredHighlightBegin = listView.preferredHighlightBegin
        listView.preferredHighlightBegin = expectedHighlightBegin
        listView.snapMode = ListView.SnapToItem
        listView.positionViewAtIndex(index, ListView.SnapPosition)
        listView.snapMode = snapMode
        listView.preferredHighlightBegin = preferredHighlightBegin
    }

    Component {
        id: sectionHeading

        Column {
            required property string section
            width: parent.width
            Rectangle {
                height: 6
                width: parent.width
                color: "transparent"
                visible: {
                    var sections
                    if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary) {
                        sections = CategorizedSortProxyModel.alphabetarySections()
                    } else if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.DDECategory) {
                        sections = CategorizedSortProxyModel.DDECategorySections()
                    }
                    if (String(sections[0]) === section.toUpperCase())
                        return false
                    else
                        return true
                }
            }

            ToolButton {
                id: headingBtn

                enabled: true
                ColorSelector.disabled: false

                focusPolicy: Qt.NoFocus
                width: parent.width

                text: {
                    if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary) {
                        return section.toUpperCase();
                    } else {
                        return getCategoryName(section)
                    }
                }

                contentItem: Item {
                    Row {
                        anchors.fill: parent
                        spacing: 3
                        QQC2.Label {
                            anchors.verticalCenter: parent.verticalCenter
                            text: headingBtn.text
                            font: LauncherController.adjustFontWeight(DTK.fontManager.t7, Font.Medium)
                            opacity: 0.6
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton

                    onClicked: {
                        if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary) {
                            alphabetCategoryPopup.existingSections = CategorizedSortProxyModel.alphabetarySections()
                            var mousePos = mapToItem(listView, mouseX, mouseY)
                            var y = (mousePos.y + alphabetCategoryPopup.height) < listView.height ? mousePos.y : listView.height - alphabetCategoryPopup.height
                            alphabetCategoryPopup.y = y
                            alphabetCategoryPopup.x = 10
                            listView.opacity = 0.1
                            alphabetCategoryPopup.open()
                        } else if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.DDECategory) {
                            ddeCategoryMenu.existingSections = CategorizedSortProxyModel.DDECategorySections()
                            listView.opacity = 0.1
                            ddeCategoryMenu.open()
                        }
                    }

                    Menu {
                        id: ddeCategoryMenu
                        width: 150
                        modal: true
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                        property var existingSections: []
                        Repeater {
                            model: ddeCategoryMenu.existingSections
                            delegate: MenuItem {
                                id: menuItem
                                text: getCategoryName(modelData)
                                onTriggered: {
                                    scrollToDDECategory(modelData)
                                }
                                contentItem: IconLabel {
                                    alignment: Qt.AlignCenter
                                    text: menuItem.text
                                    color: parent.palette.windowText
                                }
                            }
                        }

                        onVisibleChanged: {
                            if (!visible) {
                                listView.opacity = 1
                            }
                        }
                    }
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
            icon.name: (iconName && iconName !== "") ? iconName : "application-x-desktop"
            width: listView.width
            font: DTK.fontManager.t8
            palette.windowText: palette.brightText
            // icon.source: "image://app-icon/" + iconName;

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: {
                    showContextMenu(itemDelegate, model, false, false, false)
                }
            }

            Keys.onReturnPressed: {
                launchApp(desktopId)
            }

            Keys.onSpacePressed: {
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

    ListView {
        id: listView

        anchors.fill: parent
        activeFocusOnTab: true
        highlightFollowsCurrentItem: true
        // displayMarginBeginning: -45
        clip: true
        focus: true

        onActiveFocusChanged: {
            if (activeFocus) {
                // When focus in, we always scroll to the highlight
                scrollToIndex(listView.currentIndex, 0)
            }
        }

        section.property: CategorizedSortProxyModel.sortRoleName // "transliterated" // "category"
        section.criteria: section.property === "transliterated" ? ViewSection.FirstCharacter : ViewSection.FullString
        section.delegate: sectionHeading
        section.labelPositioning: ViewSection.InlineLabels // | ViewSection.CurrentLabelAtStart

        highlight: Item {
            FocusBoxBorder {
                anchors {
                    fill: parent
                    margins: 4
                }
                radius: 4
                color: palette.highlight
                visible: listView.activeFocus
            }
        }

        model: delegateCategorizedModel

        ScrollBar.vertical: ScrollBar { }

        Keys.onPressed: {
            if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary
                    && ((event.key >= Qt.Key_A && event.key <= Qt.Key_Z) || event.text === '#' || event.text === '&')) {
                scrollToAlphabetCategory(event.text.toUpperCase())
                event.accepted = true
            }
        }

        AlphabetCategoryPopup {
            id: alphabetCategoryPopup

            onCategoryClicked: {
                scrollToAlphabetCategory(character)
                close()
            }

            onVisibleChanged: {
                if (!visible) {
                    listView.opacity = 1
                }
            }
        }
    }
}
