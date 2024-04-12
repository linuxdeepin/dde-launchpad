// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0
import "."

Control {
    id: control

    property Item nextKeyTabTarget
    property Item keyTabTarget: searchResultViewContainer

    onFocusChanged: () => {
        searchResultViewContainer.focus = true
    }

    function launchCurrentItem() {
        searchResultViewContainer.currentItem?.onItemClicked()
    }

    function positionViewAtBeginning() {
        searchResultViewContainer.positionViewAtBeginning()
    }

    contentItem: ColumnLayout {
        spacing: 0
        visible: searchResultViewContainer.count > 0

        Label {
            text: qsTr("All Apps")
            font: LauncherController.adjustFontWeight(DTK.fontManager.t6, Font.Bold)
        }

        GridViewContainer {
            id: searchResultViewContainer

            KeyNavigation.tab: nextKeyTabTarget
            Layout.alignment: Qt.AlignRight
            Layout.topMargin: 10
            Layout.preferredHeight: searchResultViewContainer.height
            Layout.preferredWidth: searchResultViewContainer.width
            interactive: true

            model: SearchFilterProxyModel

            delegate: IconItemDelegate {
                width: searchResultViewContainer.cellWidth
                height: searchResultViewContainer.cellHeight
                iconSource: iconName
                onItemClicked: {
                    launchApp(desktopId)
                }
                onMenuTriggered: {
                    showContextMenu(this, model, false, false, true)
                }
            }

            activeFocusOnTab: gridViewFocus
        }

        Item {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        visible: searchResultViewContainer.count <= 0
        DciIcon {
            Layout.alignment: Qt.AlignCenter
            sourceSize {
                width: 128
                height: width
            }
            name: "search_no_result"
            palette: DTK.makeIconPalette(control.palette)
            theme: DTK.toColorType(control.palette.window)
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("No search results")
        }
    }

    background: DebugBounding { }
}
