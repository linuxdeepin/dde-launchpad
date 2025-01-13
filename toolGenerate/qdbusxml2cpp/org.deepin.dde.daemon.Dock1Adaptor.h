/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-launchpad/src/ddeintegration/xml/org.deepin.dde.daemon.Dock1.xml -a ./dde-launchpad/toolGenerate/qdbusxml2cpp/org.deepin.dde.daemon.Dock1Adaptor -i ./dde-launchpad/toolGenerate/qdbusxml2cpp/org.deepin.dde.daemon.Dock1.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef ORG_DEEPIN_DDE_DAEMON_DOCK1ADAPTOR_H
#define ORG_DEEPIN_DDE_DAEMON_DOCK1ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "./dde-launchpad/toolGenerate/qdbusxml2cpp/org.deepin.dde.daemon.Dock1.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.dde.daemon.Dock1
 */
class Dock1Adaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.daemon.Dock1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.dde.daemon.Dock1\">\n"
"    <method name=\"ActivateWindow\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"CancelPreviewWindow\"/>\n"
"    <method name=\"CloseWindow\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"GetDockedAppsDesktopFiles\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"GetEntryIDs\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"GetPluginSettings\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"IsDocked\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"IsOnDock\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"MakeWindowAbove\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"MaximizeWindow\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"MinimizeWindow\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"MoveEntry\">\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"MoveWindow\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"PreviewWindow\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"    </method>\n"
"    <method name=\"QueryWindowIdentifyMethod\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"RemovePluginSettings\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"RequestDock\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"RequestUndock\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"SetFrontendWindowRect\">\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"SetPluginSettings\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"MergePluginSettings\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <signal name=\"ServiceRestarted\"/>\n"
"    <signal name=\"EntryAdded\">\n"
"      <arg type=\"o\"/>\n"
"      <arg type=\"i\"/>\n"
"    </signal>\n"
"    <signal name=\"EntryRemoved\">\n"
"      <arg type=\"s\"/>\n"
"    </signal>\n"
"    <signal name=\"PluginSettingsSynced\"/>\n"
"    <signal name=\"DockAppSettingsSynced\"/>\n"
"    <property access=\"read\" type=\"ao\" name=\"Entries\"/>\n"
"    <property access=\"readwrite\" type=\"i\" name=\"HideMode\"/>\n"
"    <property access=\"readwrite\" type=\"i\" name=\"DisplayMode\"/>\n"
"    <property access=\"readwrite\" type=\"i\" name=\"Position\"/>\n"
"    <property access=\"readwrite\" type=\"u\" name=\"IconSize\"/>\n"
"    <property access=\"readwrite\" type=\"u\" name=\"WindowSize\"/>\n"
"    <property access=\"readwrite\" type=\"u\" name=\"WindowSizeEfficient\"/>\n"
"    <property access=\"readwrite\" type=\"u\" name=\"WindowSizeFashion\"/>\n"
"    <property access=\"read\" type=\"u\" name=\"WindowMargin\"/>\n"
"    <property access=\"readwrite\" type=\"u\" name=\"ShowTimeout\"/>\n"
"    <property access=\"readwrite\" type=\"u\" name=\"HideTimeout\"/>\n"
"    <property access=\"read\" type=\"as\" name=\"DockedApps\"/>\n"
"    <property access=\"read\" type=\"i\" name=\"HideState\"/>\n"
"    <property access=\"read\" type=\"(iiii)\" name=\"FrontendWindowRect\">\n"
"      <annotation value=\"QRect\" name=\"org.qtproject.QtDBus.QtTypeName\"/>\n"
"    </property>\n"
"    <property access=\"readwrite\" type=\"d\" name=\"Opacity\"/>\n"
"  </interface>\n"
        "")
public:
    Dock1Adaptor(QObject *parent);
    virtual ~Dock1Adaptor();

public: // PROPERTIES
    Q_PROPERTY(int DisplayMode READ displayMode WRITE setDisplayMode)
    int displayMode() const;
    void setDisplayMode(int value);

    Q_PROPERTY(QStringList DockedApps READ dockedApps)
    QStringList dockedApps() const;

    Q_PROPERTY(QList<QDBusObjectPath> Entries READ entries)
    QList<QDBusObjectPath> entries() const;

    Q_PROPERTY(QRect FrontendWindowRect READ frontendWindowRect)
    QRect frontendWindowRect() const;

    Q_PROPERTY(int HideMode READ hideMode WRITE setHideMode)
    int hideMode() const;
    void setHideMode(int value);

    Q_PROPERTY(int HideState READ hideState)
    int hideState() const;

    Q_PROPERTY(uint HideTimeout READ hideTimeout WRITE setHideTimeout)
    uint hideTimeout() const;
    void setHideTimeout(uint value);

    Q_PROPERTY(uint IconSize READ iconSize WRITE setIconSize)
    uint iconSize() const;
    void setIconSize(uint value);

    Q_PROPERTY(double Opacity READ opacity WRITE setOpacity)
    double opacity() const;
    void setOpacity(double value);

    Q_PROPERTY(int Position READ position WRITE setPosition)
    int position() const;
    void setPosition(int value);

    Q_PROPERTY(uint ShowTimeout READ showTimeout WRITE setShowTimeout)
    uint showTimeout() const;
    void setShowTimeout(uint value);

    Q_PROPERTY(uint WindowMargin READ windowMargin)
    uint windowMargin() const;

    Q_PROPERTY(uint WindowSize READ windowSize WRITE setWindowSize)
    uint windowSize() const;
    void setWindowSize(uint value);

    Q_PROPERTY(uint WindowSizeEfficient READ windowSizeEfficient WRITE setWindowSizeEfficient)
    uint windowSizeEfficient() const;
    void setWindowSizeEfficient(uint value);

    Q_PROPERTY(uint WindowSizeFashion READ windowSizeFashion WRITE setWindowSizeFashion)
    uint windowSizeFashion() const;
    void setWindowSizeFashion(uint value);

public Q_SLOTS: // METHODS
    void ActivateWindow(uint in0);
    void CancelPreviewWindow();
    void CloseWindow(uint in0);
    QStringList GetDockedAppsDesktopFiles();
    QStringList GetEntryIDs();
    QString GetPluginSettings();
    bool IsDocked(const QString &in0);
    bool IsOnDock(const QString &in0);
    void MakeWindowAbove(uint in0);
    void MaximizeWindow(uint in0);
    void MergePluginSettings(const QString &in0);
    void MinimizeWindow(uint in0);
    void MoveEntry(int in0, int in1);
    void MoveWindow(uint in0);
    void PreviewWindow(uint in0);
    QString QueryWindowIdentifyMethod(uint in0);
    void RemovePluginSettings(const QString &in0, const QStringList &in1);
    bool RequestDock(const QString &in0, int in1);
    bool RequestUndock(const QString &in0);
    void SetFrontendWindowRect(int in0, int in1, int in2, int in3);
    void SetPluginSettings(const QString &in0);
Q_SIGNALS: // SIGNALS
    void DockAppSettingsSynced();
    void EntryAdded(const QDBusObjectPath &in0, int in1);
    void EntryRemoved(const QString &in0);
    void PluginSettingsSynced();
    void ServiceRestarted();
};

#endif
