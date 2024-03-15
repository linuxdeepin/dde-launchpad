// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

pragma Singleton
import QtQuick 2.0

QtObject {
    property QtObject windowed :QtObject {
        property int topMargin: 11
        property int splitLineWidth: 1
        property int bottomBarMargins: 10
    }
    property QtObject frequentlyUsed :QtObject {
        property int rightMargin: 10
        property int cellPaddingColumns: 14
        property int cellPaddingRows: 20
    }
}
