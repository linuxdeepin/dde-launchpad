// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>

#include "appinfo.h"

class TestGioAppInfo: public QObject
{
    Q_OBJECT
private slots:
    void dfmInfo();
};

void TestGioAppInfo::dfmInfo()
{
    QString path = AppInfo::fullPathByDesktopId("dde-file-manager.desktop");
    QVERIFY(QFile::exists(path));
    QVERIFY(!path.isEmpty());
}

QTEST_MAIN(TestGioAppInfo)
#include "gioappinfotest.moc"
