// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//import QtCore
import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import org.deepin.dtk 1.0

import org.deepin.launchpad 1.0
import org.deepin.launchpad.models 1.0

Column {
    spacing: 15

    Switch {
        text: "Use regular window (need restart)"
        objectName: "UseRegularWindowNeedRestart"
        checked: DebugHelper.useRegularWindow
        onCheckedChanged: {
            DebugHelper.useRegularWindow = checked
        }
    }

    Switch {
        text: "Avoid launch application"
        objectName: "AvoidLaunchApplication"
        checked: DebugHelper.avoidLaunchApp
        onCheckedChanged: {
            DebugHelper.avoidLaunchApp = checked
        }
    }

    Switch {
        text: "Avoid hide launchpad window"
        objectName: "AvoidHideLaunchpadWindow"
        checked: DebugHelper.avoidHideWindow
        onCheckedChanged: {
            DebugHelper.avoidHideWindow = checked
        }
    }

    Switch {
        text: "Display item's bounding rectangle"
        objectName: "DisplayItemSBoundingRectangle"
        checked: DebugHelper.itemBoundingEnabled
        onCheckedChanged: {
            DebugHelper.itemBoundingEnabled = checked
        }
    }

    ToolButton {
        text: "Close launchpad application"
        objectName: "CloseLaunchpadApplication"
        onClicked: {
            Qt.quit()
        }
    }
}
