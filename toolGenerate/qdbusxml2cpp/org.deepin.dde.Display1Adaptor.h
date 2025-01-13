/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ./dde-launchpad/src/ddeintegration/xml/org.deepin.dde.Display1.xml -a ./dde-launchpad/toolGenerate/qdbusxml2cpp/org.deepin.dde.Display1Adaptor -i ./dde-launchpad/toolGenerate/qdbusxml2cpp/org.deepin.dde.Display1.h
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef ORG_DEEPIN_DDE_DISPLAY1ADAPTOR_H
#define ORG_DEEPIN_DDE_DISPLAY1ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "./dde-launchpad/toolGenerate/qdbusxml2cpp/org.deepin.dde.Display1.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.dde.Display1
 */
class Display1Adaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.Display1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.dde.Display1\">\n"
"    <method name=\"ApplyChanges\"/>\n"
"    <method name=\"AssociateTouch\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"AssociateTouchByUUID\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"ChangeBrightness\">\n"
"      <arg direction=\"in\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"DeleteCustomMode\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetRealDisplayMode\">\n"
"      <arg direction=\"out\" type=\"y\"/>\n"
"    </method>\n"
"    <method name=\"ListOutputNames\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"ModifyConfigName\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"RefreshBrightness\"/>\n"
"    <method name=\"Reset\"/>\n"
"    <method name=\"ResetChanges\"/>\n"
"    <method name=\"Save\"/>\n"
"    <method name=\"SetAndSaveBrightness\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"d\"/>\n"
"    </method>\n"
"    <method name=\"SetBrightness\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"d\"/>\n"
"    </method>\n"
"    <method name=\"SetPrimary\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"SwitchMode\">\n"
"      <arg direction=\"in\" type=\"y\"/>\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"SetMethodAdjustCCT\">\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"SetColorTemperature\">\n"
"      <arg direction=\"in\" type=\"i\"/>\n"
"    </method>\n"
"    <property access=\"read\" type=\"b\" name=\"HasChanged\"/>\n"
"    <property access=\"read\" type=\"y\" name=\"DisplayMode\"/>\n"
"    <property access=\"read\" type=\"q\" name=\"ScreenWidth\"/>\n"
"    <property access=\"read\" type=\"q\" name=\"ScreenHeight\"/>\n"
"    <property access=\"read\" type=\"s\" name=\"Primary\"/>\n"
"    <property access=\"read\" type=\"s\" name=\"CurrentCustomId\"/>\n"
"    <property access=\"read\" type=\"as\" name=\"CustomIdList\"/>\n"
"    <property access=\"read\" type=\"u\" name=\"MaxBacklightBrightness\"/>\n"
"    <property access=\"read\" type=\"(nnqq)\" name=\"PrimaryRect\">\n"
"      <annotation value=\"QRect\" name=\"org.qtproject.QtDBus.QtTypeName\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"ao\" name=\"Monitors\"/>\n"
"    <property access=\"read\" type=\"i\" name=\"ColorTemperatureMode\"/>\n"
"    <property access=\"read\" type=\"i\" name=\"ColorTemperatureManual\"/>\n"
"  </interface>\n"
        "")
public:
    Display1Adaptor(QObject *parent);
    virtual ~Display1Adaptor();

public: // PROPERTIES
    Q_PROPERTY(int ColorTemperatureManual READ colorTemperatureManual)
    int colorTemperatureManual() const;

    Q_PROPERTY(int ColorTemperatureMode READ colorTemperatureMode)
    int colorTemperatureMode() const;

    Q_PROPERTY(QString CurrentCustomId READ currentCustomId)
    QString currentCustomId() const;

    Q_PROPERTY(QStringList CustomIdList READ customIdList)
    QStringList customIdList() const;

    Q_PROPERTY(uchar DisplayMode READ displayMode)
    uchar displayMode() const;

    Q_PROPERTY(bool HasChanged READ hasChanged)
    bool hasChanged() const;

    Q_PROPERTY(uint MaxBacklightBrightness READ maxBacklightBrightness)
    uint maxBacklightBrightness() const;

    Q_PROPERTY(QList<QDBusObjectPath> Monitors READ monitors)
    QList<QDBusObjectPath> monitors() const;

    Q_PROPERTY(QString Primary READ primary)
    QString primary() const;

    Q_PROPERTY(QRect PrimaryRect READ primaryRect)
    QRect primaryRect() const;

    Q_PROPERTY(ushort ScreenHeight READ screenHeight)
    ushort screenHeight() const;

    Q_PROPERTY(ushort ScreenWidth READ screenWidth)
    ushort screenWidth() const;

public Q_SLOTS: // METHODS
    void ApplyChanges();
    void AssociateTouch(const QString &in0, const QString &in1);
    void AssociateTouchByUUID(const QString &in0, const QString &in1);
    void ChangeBrightness(bool in0);
    void DeleteCustomMode(const QString &in0);
    uchar GetRealDisplayMode();
    QStringList ListOutputNames();
    void ModifyConfigName(const QString &in0, const QString &in1);
    void RefreshBrightness();
    void Reset();
    void ResetChanges();
    void Save();
    void SetAndSaveBrightness(const QString &in0, double in1);
    void SetBrightness(const QString &in0, double in1);
    void SetColorTemperature(int in0);
    void SetMethodAdjustCCT(int in0);
    void SetPrimary(const QString &in0);
    void SwitchMode(uchar in0, const QString &in1);
Q_SIGNALS: // SIGNALS
};

#endif