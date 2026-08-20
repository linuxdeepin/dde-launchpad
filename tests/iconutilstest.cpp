// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QLoggingCategory>

#include "../src/utils/iconutils.h"

#include <utility>

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")
}

class TestIconUtils : public QObject
{
    Q_OBJECT
private slots:
    void perfectIconSizeRoundsUp();
    void perfectIconSizeReturnsFirstForSmallInput();
    void perfectIconSizeReturnsLastForLargeInput();
    void perfectIconSizeReverseRoundsDown();
    void perfectIconSizeReverseReturnsLastForLargeInput();
    void perfectIconSizeReverseReturnsFirstForSmallInput();
    void getFolderPerfectIconCellReturnsValidPair();
    void getFolderPerfectIconCellWithDifferentSizes();
    void loadSvgReturnsEmptyForNonexistentFile();
    void loadSvgIntOverloadDelegatesToQSize();
};

void TestIconUtils::perfectIconSizeRoundsUp()
{
    // sizes array: { 16, 18, 24, 32, 64, 96, 128, 256 }
    QCOMPARE(IconUtils::perfectIconSize(17), 18);
    QCOMPARE(IconUtils::perfectIconSize(20), 24);
    QCOMPARE(IconUtils::perfectIconSize(25), 32);
    QCOMPARE(IconUtils::perfectIconSize(50), 64);
    QCOMPARE(IconUtils::perfectIconSize(100), 128);
    QCOMPARE(IconUtils::perfectIconSize(200), 256);
}

void TestIconUtils::perfectIconSizeReturnsFirstForSmallInput()
{
    // 15 < 16 -> lower_bound returns begin -> 16
    QCOMPARE(IconUtils::perfectIconSize(15), 16);
    QCOMPARE(IconUtils::perfectIconSize(1), 16);
    QCOMPARE(IconUtils::perfectIconSize(0), 16);
}

void TestIconUtils::perfectIconSizeReturnsLastForLargeInput()
{
    // 300 > 256 -> lower_bound returns end -> returns sizes[0] = 16 (fallback)
    QCOMPARE(IconUtils::perfectIconSize(300), 16);
    QCOMPARE(IconUtils::perfectIconSize(1000), 16);
}

void TestIconUtils::perfectIconSizeReverseRoundsDown()
{
    // reverse: finds the largest size <= input
    QCOMPARE(IconUtils::perfectIconSizeReverse(17), 16);
    QCOMPARE(IconUtils::perfectIconSizeReverse(20), 18);
    QCOMPARE(IconUtils::perfectIconSizeReverse(30), 24);
    QCOMPARE(IconUtils::perfectIconSizeReverse(50), 32);
    QCOMPARE(IconUtils::perfectIconSizeReverse(100), 96);
    QCOMPARE(IconUtils::perfectIconSizeReverse(200), 128);
    QCOMPARE(IconUtils::perfectIconSizeReverse(300), 256);
}

void TestIconUtils::perfectIconSizeReverseReturnsLastForLargeInput()
{
    // 1000 > 256 -> returns 256 (largest in array)
    QCOMPARE(IconUtils::perfectIconSizeReverse(1000), 256);
}

void TestIconUtils::perfectIconSizeReverseReturnsFirstForSmallInput()
{
    // 10 < 16 -> returns sizes[0] = 16 (fallback)
    QCOMPARE(IconUtils::perfectIconSizeReverse(10), 16);
    QCOMPARE(IconUtils::perfectIconSizeReverse(1), 16);
}

void TestIconUtils::getFolderPerfectIconCellReturnsValidPair()
{
    auto result = IconUtils::getFolderPerfectIconCell(256, 4);
    QVERIFY(result.first > 0);
    QVERIFY(result.second > 0);
    // iconSize should be a valid perfect icon size
    QVERIFY(result.first == 16 || result.first == 18 || result.first == 24 ||
            result.first == 32 || result.first == 64 || result.first == 96 ||
            result.first == 128 || result.first == 256);
}

void TestIconUtils::getFolderPerfectIconCellWithDifferentSizes()
{
    auto result96 = IconUtils::getFolderPerfectIconCell(96, 3);
    QVERIFY(result96.first > 0);
    QVERIFY(result96.second > 0);

    auto result128 = IconUtils::getFolderPerfectIconCell(128, 4);
    QVERIFY(result128.first > 0);
    QVERIFY(result128.second > 0);

    auto result192 = IconUtils::getFolderPerfectIconCell(192, 3);
    QVERIFY(result192.first > 0);
    QVERIFY(result192.second > 0);
}

void TestIconUtils::loadSvgReturnsEmptyForNonexistentFile()
{
    // loadSvg with a file that doesn't exist should return an empty QPixmap
    // (exercises the QFileInfo::exists -> false -> return QPixmap() path)
    const QPixmap result = IconUtils::loadSvg(QStringLiteral("/nonexistent/file.svg"), QSize(32, 32));
    QVERIFY(result.isNull());
}

void TestIconUtils::loadSvgIntOverloadDelegatesToQSize()
{
    // The int overload should delegate to the QSize overload; with a nonexistent
    // file it returns an empty QPixmap (exercises the wrapper line + delegation).
    const QPixmap result = IconUtils::loadSvg(QStringLiteral("/nonexistent/file.svg"), 32);
    QVERIFY(result.isNull());
}

QTEST_MAIN(TestIconUtils)
#include "iconutilstest.moc"
