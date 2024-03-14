qt_add_qml_module(launcher-qml-windowed
    URI org.deepin.launchpad.windowed
    STATIC
    VERSION 1.0
    PLUGIN_TARGET launcher-qml-windowed
    RESOURCE_PREFIX
        /qt/qml
    QML_FILES
        SideBar.qml
        AlphabetCategoryPopup.qml
        AppList.qml
        AppListView.qml
        IconItemDelegate.qml
        WindowedFrame.qml
)
