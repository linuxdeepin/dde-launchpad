# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

qt_add_qml_module(launcher-qml-windowed
    URI org.deepin.launchpad.windowed
    STATIC
    VERSION 1.0
    PLUGIN_TARGET launcher-qml-windowed
    RESOURCE_PREFIX
        /qt/qml
    QML_FILES
        WindowedFrame.qml
        SideBar.qml
        BottomBar.qml
        AlphabetCategoryPopup.qml
        AppList.qml
        AppListView.qml
        IconItemDelegate.qml
        FreeSortListView.qml
        GridViewContainer.qml
        AnalysisView.qml
        RecentlyInstalledView.qml
        FrequentlyUsedView.qml
)
