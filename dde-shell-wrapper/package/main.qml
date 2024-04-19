// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import org.deepin.ds 1.0
import org.deepin.launchpad 1.0

ContainmentItem {
    id: root
    property point windowedPos: Qt.point(0, 0)
    Main {
        windowedPos: root.windowedPos
    }

    function hide() {
        LauncherController.visible = false
    }
}
