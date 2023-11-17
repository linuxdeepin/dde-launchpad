// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15 as QQC2
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

Item {
    id: root

    property alias model: listView.model

    signal sectionHeaderClicked(var categoryType)

    Timer {
        id: postScrollDeferTimer
        interval: 150 // same as the defaul value of ListView's highlightMoveDuration
        onTriggered: {
            // if we set back the highlightRangeMode right after we set currentIndex,
            // the scroll might stopped before the highlight item scroll to the expected place.
            // Thus we use a timer to set back these values with a delay.
            listView.highlightMoveDuration = 150
            listView.highlightRangeMode = GridView.NoHighlightRange
        }
    }

    function scrollToAlphabetCategory(character) {
        for (let i = 0; i < model.count; i++) {
            let transliterated1st = model.model.data(model.modelIndex(i), 4096)[0].toUpperCase() // 4096 is AppsModel::TransliteratedRole
            if (character === transliterated1st) {
                // we use the highlight move to scroll to item
                listView.highlightMoveDuration = 0
                listView.highlightRangeMode = GridView.ApplyRange
                listView.currentIndex = i
                postScrollDeferTimer.restart()
                break
            }
        }
    }

    Component {
        id: sectionHeading
        ToolButton {
            required property string section

            enabled: CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary

            id: headingBtn
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
                    }
                }
            }

            onClicked: {
                sectionHeaderClicked(CategorizedSortProxyModel.categoryType)
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

        section.property: CategorizedSortProxyModel.sortRoleName // "transliterated" // "category"
        section.criteria: section.property === "transliterated" ? ViewSection.FirstCharacter : ViewSection.FullString
        section.delegate: sectionHeading
        section.labelPositioning: ViewSection.InlineLabels // | ViewSection.CurrentLabelAtStart
        preferredHighlightBegin: 35 // the height of a section heading
        preferredHighlightEnd: 35

        model: visualModel

        highlight: Item {
            SystemPalette { id: highlightPalette }
            FocusBoxBorder {
                anchors {
                    fill: parent
                    margins: 5
                }
                radius: 8
                color: highlightPalette.highlight
                visible: listView.activeFocus
            }
        }

        ScrollBar.vertical: ScrollBar { }

        Keys.onPressed: {
            if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary
                    && ((event.key >= Qt.Key_A && event.key <= Qt.Key_Z) || event.text === '#' || event.text === '&')) {
                scrollToAlphabetCategory(event.text.toUpperCase())
                event.accepted = true
            }
        }
    }
}
