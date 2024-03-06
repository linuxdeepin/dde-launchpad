// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>
#include <QtQml/qqml.h>

class QSettings;
class DebugHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool qtDebugEnabled READ qtDebugEnabled CONSTANT)
    Q_PROPERTY(bool useRegularWindow MEMBER m_useRegularWindow NOTIFY onUseRegularWindowChanged)
    Q_PROPERTY(bool avoidLaunchApp MEMBER m_avoidLaunchApp NOTIFY onAvoidLaunchAppChanged)
    Q_PROPERTY(bool avoidHideWindow MEMBER m_avoidHideWindow NOTIFY onAvoidHideWindowChanged)
    QML_NAMED_ELEMENT(DebugHelper)
    QML_SINGLETON
public:
    static DebugHelper &instance()
    {
        static DebugHelper _instance;
        return _instance;
    }

    static DebugHelper *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
    {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &instance();
    }

    ~DebugHelper();

    bool qtDebugEnabled() const;

signals:
    void onUseRegularWindowChanged(bool);
    void onAvoidLaunchAppChanged(bool);
    void onAvoidHideWindowChanged(bool);

private:
    explicit DebugHelper(QObject * parent = nullptr);

    QSettings * m_debugSettings;
    bool m_useRegularWindow;
    bool m_avoidLaunchApp;
    bool m_avoidHideWindow;
};
