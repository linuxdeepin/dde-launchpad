// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Controls 2.15 as QQC2
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0

QQC2.Control {
    id: root

    property alias model: listView.model

    Component {
        id: sectionHeading
        Rectangle {
            width: parent.width
            height: childrenRect.height
            color: "transparent"

            required property string section

            QQC2.Label {
                text: {
                    if (CategorizedSortProxyModel.categoryType === CategorizedSortProxyModel.Alphabetary) {
                        return parent.section.toUpperCase();
                    } else {
                        switch (Number(parent.section)) {
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
                font.bold: true
                font.pixelSize: 15
            }
        }
    }

    contentItem: ListView {
        id: listView

        activeFocusOnTab: true
        highlightFollowsCurrentItem: true
        // displayMarginBeginning: -45
        clip: true
        focus: true

        section.property: CategorizedSortProxyModel.sortRoleName // "transliterated" // "category"
        section.criteria: section.property === "transliterated" ? ViewSection.FirstCharacter : ViewSection.FullString
        section.delegate: sectionHeading
        section.labelPositioning: ViewSection.InlineLabels | ViewSection.CurrentLabelAtStart

        model: visualModel

        highlight: Rectangle {
            color: "transparent"
            FocusBoxBorder {
                anchors {
                    fill: parent
                    margins: 5
                }
                radius: 8
                color: root.palette.highlight // TODO: if `SystemPalette` works, we can change the root item back from Control to Item
                visible: listView.activeFocus
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }
}
