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

    function scrollToAlphabetCategory(character) {
        for (let i = 0; i < model.count; i++) {
            let transliterated1st = model.model.data(model.modelIndex(i), 4096)[0].toUpperCase() // 4096 is AppsModel::TransliteratedRole
            if (character === transliterated1st) {
                // we use the highlight move to scroll to item
                listView.highlightMoveDuration = 0
                listView.highlightRangeMode = GridView.ApplyRange
                listView.currentIndex = i
                listView.highlightMoveDuration = 150
                listView.highlightRangeMode = GridView.NoHighlightRange
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
            width: parent.width
            text: {
                if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary) {
                    return section.toUpperCase();
                } else {
                    switch (Number(section)) {
                    case AppItem.Internet:
                        return qsTr("Internet");
                    case AppItem.Chat:
                        return qsTr("Chat");
                    case AppItem.Music:
                        return qsTr("Music");
                    case AppItem.Video:
                        return qsTr("Video");
                    case AppItem.Graphics:
                        return qsTr("Graphics");
                    case AppItem.Game:
                        return qsTr("Game");
                    case AppItem.Office:
                        return qsTr("Office");
                    case AppItem.Reading:
                        return qsTr("Reading");
                    case AppItem.Development:
                        return qsTr("Development");
                    case AppItem.System:
                        return qsTr("System");
                    default:
                        return qsTr("Others");
                    }
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
    }
}
