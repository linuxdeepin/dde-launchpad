// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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

    property Item keyTabTarget: listView
    property alias model: listView.model
    property var categoryMenu: null

    function resetViewState() {
        // 临时禁用highlightFollowsCurrentItem以避免动画
        let wasFollowing = listView.highlightFollowsCurrentItem
        listView.highlightFollowsCurrentItem = false
        listView.currentIndex = 0
        listView.contentY = 0
        listView.highlightFollowsCurrentItem = wasFollowing
        if (!LauncherController.visible) {
            alphabetCategoryPopup.close()
            if (categoryMenu) categoryMenu.close()
        }
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
                                alphabetCategoryPopup.setCurrentCategory(section.toUpperCase())
                                var mousePos = mapToItem(listView, mouseX, mouseY)
                                var y = (mousePos.y + alphabetCategoryPopup.height) < listView.height ? mousePos.y : listView.height - alphabetCategoryPopup.height
                                alphabetCategoryPopup.y = y
                                alphabetCategoryPopup.x = 10
                                listView.opacity = 0.1
                                alphabetCategoryPopup.open()
                            } else if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.DDECategory) {
                                ddeCategoryMenu.existingSections = CategorizedSortProxyModel.DDECategorySections()
                                listView.opacity = 0.1
                                root.categoryMenu = ddeCategoryMenu
                                ddeCategoryMenu.popup(headingBtn, 0, 0)
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

        delegate: Item {
            width: listView.width
            height: itemDelegate.height
            ItemDelegate {
                id: itemDelegate
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: 10
                    rightMargin: 10
                }
                opacity: !Drag.active ? 1 : 0
                text: model.display
                checkable: false
                icon.name: (iconName && iconName !== "") ? iconName : "application-x-desktop"
                DciIcon.mode: DTK.NormalState
                DciIcon {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    name: "emblem_autostart"
                    visible: autoStart
                    sourceSize: Qt.size(12, 12)
                    palette: DTK.makeIconPalette(parent.palette)
                    theme: ApplicationHelper.DarkType
                    z: 1
                }
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: DTK.themeType === ApplicationHelper.LightType ? "#226BEF" : "#669DFF"
                    border.width: 1
                    border.color: DTK.themeType === ApplicationHelper.LightType ?
                                 Qt.rgba(0, 0, 0, 0.1) : Qt.rgba(1, 1, 1, 0.1)
                    visible: model.lastLaunchedTime === 0 && model.installedTime !== 0
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: 6
                        rightMargin: 3
                    }
                    z: 1
                }
                font: DTK.fontManager.t8
                ColorSelector.pressed: false
                property Palette textColor: DStyle.Style.button.text
                palette.windowText: ColorSelector.textColor
                ToolTip.text: text
                ToolTip.delay: 500
                ToolTip.visible: hovered && contentItem.implicitWidth > contentItem.width

                Drag.dragType: Drag.Automatic
                Drag.active: mouseArea.drag.active
                Drag.mimeData: Helper.generateDragMimeData(model.desktopId, true)
                Drag.hotSpot.y: height / 2
                Drag.hotSpot.x: width / 2

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    drag.target: itemDelegate
                    // 当分类菜单打开时，禁用拖拽功能
                    enabled: !(ddeCategoryMenu.visible || alphabetCategoryPopup.visible)

                    onPressed: function (mouse) {
                        if (mouse.button === Qt.LeftButton) {
                            itemDelegate.contentItem.grabToImage(function(result) {
                                itemDelegate.Drag.imageSource = result.url
                            })
                        }
                    }
                    onClicked: function (mouse) {
                        if (mouse.button === Qt.RightButton) {
                            showContextMenu(itemDelegate, model)
                            baseLayer.focus = true
                        } else {
                            launchApp(desktopId)
                        }
                    }
                }
                background: ItemBackground {
                    implicitWidth: DStyle.Style.itemDelegate.width
                    implicitHeight: Helper.windowed.listItemHeight
                    button: itemDelegate
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
                background: BoxPanel {
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    visible: menuItem.highlighted
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

    ListView {
        id: listView

        anchors.fill: parent
        highlightFollowsCurrentItem: true
        // displayMarginBeginning: -45
        clip: true
        focus: true

        onActiveFocusChanged: {
            if (activeFocus) {
                // When focus in, we always scroll to the highlight
                scrollToIndex(listView.currentIndex, 0)
                if(currentItem)
                    currentItem.forceActiveFocus()
            }
        }

        Connections {
            target: CategorizedSortProxyModel
            function onCategoryTypeChanged() {
                listView.currentIndex = 0
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

        // End margin, see PMS-301113
        footer: Item {
            height: 10
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
