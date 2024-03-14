// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Item {
    id: root

    width: 200
    height: 200

    required property DelegateModel model
    property alias currentIndex: view.currentIndex

    function incrementCurrentIndex() { view.incrementCurrentIndex() }
    function decrementCurrentIndex() { view.decrementCurrentIndex() }

    PathView {
        id: view

        anchors.fill: parent

        snapMode: PathView.SnapOneItem
        highlightRangeMode: PathView.StrictlyEnforceRange
        currentIndex: -1
        pathItemCount: Math.min(3, root.model.count)

        model: root.model.model
        delegate: root.model.delegate

        path: Path {
            startX: -view.width / 2
            startY: view.height / 2

            PathLine {
                relativeX: view.width * view.pathItemCount
                relativeY: 0
            }
        }
    }
}
