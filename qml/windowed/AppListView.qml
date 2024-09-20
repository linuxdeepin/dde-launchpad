// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15 as QQC2
import org.deepin.dtk 1.0
import org.deepin.dtk.private 1.0
import org.deepin.dtk.style 1.0 as DStyle

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

FocusScope {
    id: root

    property Item nextKeyTabTargetItem
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

            Item {
                width: parent.width
                height: headingBtn.height
                ToolButton {
                    id: headingBtn

                    enabled: true
                    ColorSelector.disabled: false

                    focusPolicy: Qt.NoFocus
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10

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

                    background: ItemBackground {
                        button: headingBtn
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
                                    textColor: DStyle.Style.menu.itemText
                                    onTriggered: {
                                        scrollToDDECategory(modelData)
                                    }
                                    contentItem: IconLabel {
                                        alignment: Qt.AlignCenter
                                        text: menuItem.text
                                        color: parent.palette.windowText
                                    }
                                    background: ButtonPanel {
                                        button: parent
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        anchors.right: parent.right
                                        anchors.rightMargin: 10
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        visible: menuItem.down || menuItem.highlighted
                                        outsideBorderColor: null
                                        insideBorderColor: null
                                        radius: 6

                                        property Palette background: Palette {
                                            normal {
                                                common: Qt.rgba(0, 0, 0, 0.1)
                                                crystal: Qt.rgba(0, 0, 0, 0.1)
                                            }
                                            normalDark {
                                                common: Qt.rgba(1, 1, 1, 0.1)
                                                crystal: Qt.rgba(1, 1, 1, 0.1)
                                            }
                                            hovered {
                                                common: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.1)
                                                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.1)
                                            }
                                        }
                                        color1: background
                                        color2: background
                                    }
                                }
                            }

                            onVisibleChanged: {
                                if (!visible) {
                                    listView.opacity = 1
                                }
                            }
                            background: FloatingPanel {
                                radius: DStyle.Style.menu.radius
                                backgroundColor: ddeCategoryMenu.backgroundColor
                                backgroundNoBlurColor: ddeCategoryMenu.backgroundNoBlurColor
                                dropShadowColor: null
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
            width: root.width
            KeyNavigation.tab: nextKeyTabTargetItem

            text: model.display
            checkable: false
            icon.name: (iconName && iconName !== "") ? iconName : "application-x-desktop"
            DciIcon.mode: DTK.NormalState
            font: DTK.fontManager.t8
            palette.windowText: ListView.view.palette.brightText
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            ToolTip.text: text
            ToolTip.delay: 500
            ToolTip.visible: hovered && contentItem.implicitWidth > contentItem.width

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: {
                    showContextMenu(itemDelegate, model)
                    baseLayer.focus = true
                }
            }

            background: Loader {
                active: itemDelegate.ListView.view.bgVisible === undefined
                sourceComponent: ItemBackground {
                    implicitWidth: DStyle.Style.itemDelegate.width
                    implicitHeight: Helper.windowed.listItemHeight
                    button: itemDelegate
                }
            }

            TapHandler {
                onTapped: {
                    launchApp(desktopId)
                }
            }

            Keys.onReturnPressed: {
                launchApp(desktopId)
            }

            Keys.onSpacePressed: {
                launchApp(desktopId)
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
            focus: false
            FocusBoxBorder {
                anchors {
                    fill: parent
                    margins: 4
                }
                radius: 4
                color: parent.palette.highlight
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
