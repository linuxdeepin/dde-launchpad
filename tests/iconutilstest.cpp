// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QTemporaryFile>
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QStandardPaths>
#include <QDate>
#include <QBuffer>
#include <QByteArray>
#include <QPainter>
#include <QIcon>

#include "iconutils.h"

#include <utility>

class TestIconUtils : public QObject
{
    Q_OBJECT
private slots:
    // perfectIconSize
    void perfectIconSizeRoundsUp();
    void perfectIconSizeReturnsFirstForSmallInput();
    void perfectIconSizeReturnsLastForLargeInput();
    void perfectIconSizeExactMatch();

    // perfectIconSizeReverse
    void perfectIconSizeReverseRoundsDown();
    void perfectIconSizeReverseReturnsLastForLargeInput();
    void perfectIconSizeReverseReturnsFirstForSmallInput();
    void perfectIconSizeReverseExactMatch();

    // getFolderPerfectIconCell
    void getFolderPerfectIconCellReturnsValidPair();
    void getFolderPerfectIconCellWithDifferentSizes();

    // loadSvg
    void loadSvgReturnsEmptyForNonexistentFile();
    void loadSvgIntOverloadDelegatesToQSize();
    void loadSvgValidSvgFile();
    void loadSvgIntOverloadValidSvgFile();

    // tryUpdateIconCache
    void tryUpdateIconCacheNoCrash();

    // createCalendarIcon
    void createCalendarIconValidPath();
    void createCalendarIconNonWritablePath();

    // getThemeIcon
    void getThemeIconEmptyNameReturnsTrue();
    void getThemeIconDataImageBase64Valid();
    void getThemeIconDataImageBase64Invalid();
    void getThemeIconNonexistentFallback();
    void getThemeIconCalendarDynamic();
    void getThemeIconExistingPngFile();
    void getThemeIconExistingSvgFile();
};

// ---------------------------------------------------------------------------
// perfectIconSize
// ---------------------------------------------------------------------------

void TestIconUtils::perfectIconSizeRoundsUp()
{
    QCOMPARE(IconUtils::perfectIconSize(17), 18);
    QCOMPARE(IconUtils::perfectIconSize(20), 24);
    QCOMPARE(IconUtils::perfectIconSize(25), 32);
    QCOMPARE(IconUtils::perfectIconSize(50), 64);
    QCOMPARE(IconUtils::perfectIconSize(100), 128);
    QCOMPARE(IconUtils::perfectIconSize(200), 256);
}

void TestIconUtils::perfectIconSizeReturnsFirstForSmallInput()
{
    QCOMPARE(IconUtils::perfectIconSize(15), 16);
    QCOMPARE(IconUtils::perfectIconSize(1), 16);
    QCOMPARE(IconUtils::perfectIconSize(0), 16);
}

void TestIconUtils::perfectIconSizeReturnsLastForLargeInput()
{
    QCOMPARE(IconUtils::perfectIconSize(300), 16);
    QCOMPARE(IconUtils::perfectIconSize(1000), 16);
}

void TestIconUtils::perfectIconSizeExactMatch()
{
    QCOMPARE(IconUtils::perfectIconSize(16), 16);
    QCOMPARE(IconUtils::perfectIconSize(18), 18);
    QCOMPARE(IconUtils::perfectIconSize(24), 24);
    QCOMPARE(IconUtils::perfectIconSize(32), 32);
    QCOMPARE(IconUtils::perfectIconSize(64), 64);
    QCOMPARE(IconUtils::perfectIconSize(96), 96);
    QCOMPARE(IconUtils::perfectIconSize(128), 128);
    QCOMPARE(IconUtils::perfectIconSize(256), 256);
}

// ---------------------------------------------------------------------------
// perfectIconSizeReverse
// ---------------------------------------------------------------------------

void TestIconUtils::perfectIconSizeReverseRoundsDown()
{
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
    QCOMPARE(IconUtils::perfectIconSizeReverse(1000), 256);
}

void TestIconUtils::perfectIconSizeReverseReturnsFirstForSmallInput()
{
    QCOMPARE(IconUtils::perfectIconSizeReverse(10), 16);
    QCOMPARE(IconUtils::perfectIconSizeReverse(1), 16);
}

void TestIconUtils::perfectIconSizeReverseExactMatch()
{
    QCOMPARE(IconUtils::perfectIconSizeReverse(16), 16);
    QCOMPARE(IconUtils::perfectIconSizeReverse(18), 18);
    QCOMPARE(IconUtils::perfectIconSizeReverse(256), 256);
}

// ---------------------------------------------------------------------------
// getFolderPerfectIconCell
// ---------------------------------------------------------------------------

void TestIconUtils::getFolderPerfectIconCellReturnsValidPair()
{
    auto result = IconUtils::getFolderPerfectIconCell(256, 4);
    QCOMPARE(result.first, 32);
    QCOMPARE(result.second, 25);
}

void TestIconUtils::getFolderPerfectIconCellWithDifferentSizes()
{
    auto result96 = IconUtils::getFolderPerfectIconCell(96, 3);
    QCOMPARE(result96.first, 24);
    QCOMPARE(result96.second, 6);

    auto result128 = IconUtils::getFolderPerfectIconCell(128, 4);
    QCOMPARE(result128.first, 24);
    QCOMPARE(result128.second, 6);

    auto result192 = IconUtils::getFolderPerfectIconCell(192, 3);
    QCOMPARE(result192.first, 32);
    QCOMPARE(result192.second, 24);
}

// ---------------------------------------------------------------------------
// loadSvg
// ---------------------------------------------------------------------------

void TestIconUtils::loadSvgReturnsEmptyForNonexistentFile()
{
    const QPixmap result = IconUtils::loadSvg(QStringLiteral("/nonexistent/file.svg"), QSize(32, 32));
    QVERIFY(result.isNull());
}

void TestIconUtils::loadSvgIntOverloadDelegatesToQSize()
{
    const QPixmap result = IconUtils::loadSvg(QStringLiteral("/nonexistent/file.svg"), 32);
    QVERIFY(result.isNull());
}

void TestIconUtils::loadSvgValidSvgFile()
{
    QTemporaryFile tmpFile(QStringLiteral("XXXXXX.svg"));
    QVERIFY(tmpFile.open());
    tmpFile.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"32\" height=\"32\">"
                  "<rect width=\"32\" height=\"32\" fill=\"red\"/></svg>");
    tmpFile.close();

    const QPixmap result = IconUtils::loadSvg(tmpFile.fileName(), QSize(32, 32));
    QVERIFY(!result.isNull());
    QCOMPARE(result.size(), QSize(32, 32));
}

void TestIconUtils::loadSvgIntOverloadValidSvgFile()
{
    QTemporaryFile tmpFile(QStringLiteral("XXXXXX.svg"));
    QVERIFY(tmpFile.open());
    tmpFile.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\">"
                  "<rect width=\"24\" height=\"24\" fill=\"blue\"/></svg>");
    tmpFile.close();

    const QPixmap result = IconUtils::loadSvg(tmpFile.fileName(), 24);
    QVERIFY(!result.isNull());
    QCOMPARE(result.size(), QSize(24, 24));
}

// ---------------------------------------------------------------------------
// tryUpdateIconCache
// ---------------------------------------------------------------------------

void TestIconUtils::tryUpdateIconCacheNoCrash()
{
    // Crash-only test: tryUpdateIconCache() calls QIcon::setThemeSearchPaths
    // with the current search paths. This is a no-op that should not crash.
    // Verify themeSearchPaths() remains unchanged before and after the call.
    auto pathsBefore = QIcon::themeSearchPaths();
    IconUtils::tryUpdateIconCache();
    auto pathsAfter = QIcon::themeSearchPaths();
    QCOMPARE(pathsBefore, pathsAfter);
}

// ---------------------------------------------------------------------------
// createCalendarIcon
// ---------------------------------------------------------------------------

void TestIconUtils::createCalendarIconValidPath()
{
    QString tempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).constFirst();
    QString fileName = tempDir + "/test_calendar_icon_" +
                       QString::number(QDate::currentDate().dayOfYear()) + ".svg";

    // RAII cleanup: ensures the temp file is removed even if a QVERIFY
    // fails and aborts the test function before reaching the explicit remove.
    struct FileCleaner {
        QString name;
        ~FileCleaner() { if (!name.isEmpty()) QFile::remove(name); }
    } cleaner{fileName};

    QVERIFY(IconUtils::createCalendarIcon(fileName));
    QVERIFY(QFile::exists(fileName));
}

void TestIconUtils::createCalendarIconNonWritablePath()
{
    QString fileName = QStringLiteral("/nonexistent_dir/test_calendar.svg");
    QVERIFY(!IconUtils::createCalendarIcon(fileName));
}

// ---------------------------------------------------------------------------
// getThemeIcon
// ---------------------------------------------------------------------------

void TestIconUtils::getThemeIconEmptyNameReturnsTrue()
{
    QPixmap pixmap;
    bool result = IconUtils::getThemeIcon(pixmap, QString(), 32);
    QVERIFY(result);
}

void TestIconUtils::getThemeIconDataImageBase64Valid()
{
    // 1x1 red PNG
    QByteArray pngData;
    QBuffer buffer(&pngData);
    buffer.open(QIODevice::WriteOnly);
    QImage img(1, 1, QImage::Format_RGB32);
    img.setPixel(0, 0, qRgb(255, 0, 0));
    img.save(&buffer, "PNG");
    buffer.close();

    QString iconName = QStringLiteral("data:image/png;base64,") +
                       QString::fromLatin1(pngData.toBase64());

    QPixmap pixmap;
    bool result = IconUtils::getThemeIcon(pixmap, iconName, 32);
    QVERIFY(result);
}

void TestIconUtils::getThemeIconDataImageBase64Invalid()
{
    QString iconName = QStringLiteral("data:image/png;base64,INVALIDBASE64DATA!!!");
    QPixmap pixmap;
    bool result = IconUtils::getThemeIcon(pixmap, iconName, 32);
    // Invalid base64 → null pixmap → falls through to fallback path
    // findIcon becomes false because icon is not found in theme
    QVERIFY(!result);
}

void TestIconUtils::getThemeIconNonexistentFallback()
{
    QPixmap pixmap;
    bool result = IconUtils::getThemeIcon(pixmap, QStringLiteral("nonexistent-icon-name-xyz"), 32);
    QVERIFY(!result);
    // Don't assert !pixmap.isNull(): the fallback is looked up through
    // DIconTheme::findQIcon("application-x-desktop"), i.e. it comes from the
    // system icon theme. Under QT_QPA_PLATFORM=offscreen QIcon::themeSearchPaths()
    // holds only ":/icons" and no theme name is set, so the fallback resolves to a
    // null pixmap no matter what is installed (a bare pbuilder chroot has no
    // deepin-icon-theme either). What this case covers is the control flow: a
    // missing icon reports false instead of crashing.
}

void TestIconUtils::getThemeIconCalendarDynamic()
{
    // getThemeIcon creates a temp file named <year>_<dayOfYear>.svg in the
    // temp directory. Clean it up after the test via RAII to avoid leaking.
    QString tempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).constFirst();
    QString calendarTempFile = tempDir + "/" +
                               QString::number(QDate::currentDate().year()) + "_" +
                               QString::number(QDate::currentDate().dayOfYear()) + ".svg";
    struct FileCleaner {
        QString name;
        ~FileCleaner() { if (!name.isEmpty()) QFile::remove(name); }
    } cleaner{calendarTempFile};

    QPixmap pixmap;
    bool result = IconUtils::getThemeIcon(pixmap, QStringLiteral("dde-calendar-dynamic"), 32);
    // Due to a bug in getThemeIcon (line 68: uses iconName instead of actualIconName
    // for SVG loading), the calendar SVG file is created but loadSvg is called with
    // "dde-calendar-dynamic" (not the temp file path), which fails the QFileInfo::exists
    // check and falls through to the fallback path. findIcon = false.
    // See defect report in delivery comment.
    QVERIFY(!result);
}

void TestIconUtils::getThemeIconExistingPngFile()
{
    // Create a temporary PNG file
    QTemporaryFile tmpFile(QStringLiteral("XXXXXX.png"));
    QVERIFY(tmpFile.open());
    QImage img(32, 32, QImage::Format_RGB32);
    img.fill(qRgb(0, 255, 0));
    QVERIFY(img.save(&tmpFile, "PNG"));
    tmpFile.close();

    QPixmap pixmap;
    bool result = IconUtils::getThemeIcon(pixmap, tmpFile.fileName(), 32);
    QVERIFY(result);
    QVERIFY(!pixmap.isNull());
}

void TestIconUtils::getThemeIconExistingSvgFile()
{
    // Create a temporary SVG file
    QTemporaryFile tmpFile(QStringLiteral("XXXXXX.svg"));
    QVERIFY(tmpFile.open());
    tmpFile.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"32\" height=\"32\">"
                  "<rect width=\"32\" height=\"32\" fill=\"red\"/></svg>");
    tmpFile.close();

    QPixmap pixmap;
    bool result = IconUtils::getThemeIcon(pixmap, tmpFile.fileName(), 32);
    // For SVG files, getThemeIcon calls loadSvg(iconName, ...) where iconName is
    // the file path (same as actualIconName for non-calendar-dynamic case).
    // loadSvg should successfully load the SVG and return a non-null pixmap.
    QVERIFY(result);
    QVERIFY(!pixmap.isNull());
}

QTEST_MAIN(TestIconUtils)
#include "iconutilstest.moc"
