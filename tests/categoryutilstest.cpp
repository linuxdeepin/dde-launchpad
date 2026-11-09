// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>

#include "categoryutils.h"

namespace {

using C = CategoryUtils::Categorytype;
}

class TestCategoryUtils : public QObject
{
    Q_OBJECT
private slots:
    // parseDDECategoryString
    void parseDDECategoryStringKnown();
    void parseDDECategoryStringAllKnown();
    void parseDDECategoryStringUnknown();
    void parseDDECategoryStringEmpty();

    // parseXdgCategoryString
    void parseXdgCategoryStringSingle();
    void parseXdgCategoryStringMultiValued();
    void parseXdgCategoryStringUnknown();
    void parseXdgCategoryStringAudioMapsToMusic();
    void parseXdgCategoryStringAllGraphicsEntries();
    void parseXdgCategoryStringAllGameEntries();
    void parseXdgCategoryStringAllOfficeEntries();
    void parseXdgCategoryStringAllSystemEntries();
    void parseXdgCategoryStringAllInternetEntries();
    void parseXdgCategoryStringAllChatEntries();
    void parseXdgCategoryStringAllMusicEntries();
    void parseXdgCategoryStringAllVideoEntries();
    void parseXdgCategoryStringAllReadingEntries();
    void parseXdgCategoryStringAllDevelopmentEntries();
    void parseXdgCategoryStringAllOthersEntries();
    void parseXdgCategoryStringMultipleDistinctCategories();

    // parseBestMatchedCategory
    void bestMatchedSingleCategory();
    void bestMatchedEmptyReturnsOthers();
    void bestMatchedOthersOnlyReturnsOthers();
    void bestMatchedMusicVideoTiePrefersVideo();
    void bestMatchedPlayerMapsToVideo();
    void bestMatchedMaxCountWins();
    void bestMatchedMultipleInternet();
    void bestMatchedDDECategoryWinsOverXdg();
    void bestMatchedGameVariants();
    void bestMatchedDevelopmentVariants();
    void bestMatchedReadingVariants();
    void bestMatchedSystemVariants();
    void bestMatchedGraphicsVariants();
    void bestMatchedChatVariants();
    void bestMatchedOfficeVariants();
    void bestMatchedMixedUnknownAndKnown();
    void bestMatchedAudioVideoEditingTie();
    void bestMatchedRecorderTie();
    void bestMatchedNonMusicVideoTieReturnsSortedFirst();
    void bestMatchedAccessoriesRemovedReturnsOthers();
    void bestMatchedAllUnknownReturnsOthers();
    void bestMatchedDDEAndXdgSameTypeAccumulates();
    void bestMatchedThreeWayTie();
    void bestMatchedCaseInsensitive();
};

// ---------------------------------------------------------------------------
// parseDDECategoryString
// ---------------------------------------------------------------------------

void TestCategoryUtils::parseDDECategoryStringKnown()
{
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("music"))), int(C::CategoryMusic));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("internet"))), int(C::CategoryInternet));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("development"))), int(C::CategoryDevelopment));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("others"))), int(C::CategoryOthers));
}

void TestCategoryUtils::parseDDECategoryStringAllKnown()
{
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("internet"))), int(C::CategoryInternet));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("chat"))), int(C::CategoryChat));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("music"))), int(C::CategoryMusic));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("video"))), int(C::CategoryVideo));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("graphics"))), int(C::CategoryGraphics));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("office"))), int(C::CategoryOffice));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("game"))), int(C::CategoryGame));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("reading"))), int(C::CategoryReading));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("development"))), int(C::CategoryDevelopment));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("system"))), int(C::CategorySystem));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("others"))), int(C::CategoryOthers));
}

void TestCategoryUtils::parseDDECategoryStringUnknown()
{
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("unknown"))), int(C::CategoryErr));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("Music"))), int(C::CategoryErr));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("AUDIO"))), int(C::CategoryErr));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("xyz"))), int(C::CategoryErr));
}

void TestCategoryUtils::parseDDECategoryStringEmpty()
{
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral(""))), int(C::CategoryErr));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QString())), int(C::CategoryErr));
}

// ---------------------------------------------------------------------------
// parseXdgCategoryString
// ---------------------------------------------------------------------------

void TestCategoryUtils::parseXdgCategoryStringSingle()
{
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("webbrowser"));
    QCOMPARE(result.size(), 1);
    QVERIFY(result.contains(C::CategoryInternet));
}

void TestCategoryUtils::parseXdgCategoryStringMultiValued()
{
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("audiovideo"));
    QCOMPARE(result.size(), 2);
    QVERIFY(result.contains(C::CategoryMusic));
    QVERIFY(result.contains(C::CategoryVideo));
}

void TestCategoryUtils::parseXdgCategoryStringUnknown()
{
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("totally-unknown-category"));
    QVERIFY(result.isEmpty());
}

void TestCategoryUtils::parseXdgCategoryStringAudioMapsToMusic()
{
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("audio"));
    QCOMPARE(result.size(), 1);
    QVERIFY(result.contains(C::CategoryMusic));
}

void TestCategoryUtils::parseXdgCategoryStringAllGraphicsEntries()
{
    const QStringList keys = {
        QStringLiteral("2dgraphics"), QStringLiteral("3dgraphics"),
        QStringLiteral("graphics"), QStringLiteral("imageprocessing"),
        QStringLiteral("ocr"), QStringLiteral("photography"),
        QStringLiteral("rastergraphics"), QStringLiteral("vectorgraphics"),
        QStringLiteral("viewer"), QStringLiteral("x-geeqie"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryGraphics), qPrintable(QStringLiteral("Missing Graphics for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllGameEntries()
{
    const QStringList keys = {
        QStringLiteral("game"), QStringLiteral("actiongame"),
        QStringLiteral("adventuregame"), QStringLiteral("amusement"),
        QStringLiteral("arcadegame"), QStringLiteral("blocksgame"),
        QStringLiteral("boardgame"), QStringLiteral("cardgame"),
        QStringLiteral("emulator"), QStringLiteral("kidsgame"),
        QStringLiteral("logicgame"), QStringLiteral("puzzlegame"),
        QStringLiteral("roleplaying"), QStringLiteral("simulation"),
        QStringLiteral("sportsgame"), QStringLiteral("strategygame"),
        QStringLiteral("x-debian-applications-emulators"),
        QStringLiteral("x-suse-core-game"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryGame), qPrintable(QStringLiteral("Missing Game for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllOfficeEntries()
{
    const QStringList keys = {
        QStringLiteral("office"), QStringLiteral("art"),
        QStringLiteral("artificialintelligence"), QStringLiteral("astronomy"),
        QStringLiteral("biology"), QStringLiteral("chart"),
        QStringLiteral("chemistry"), QStringLiteral("computerscience"),
        QStringLiteral("dictionary"), QStringLiteral("documentation"),
        QStringLiteral("education"), QStringLiteral("electricity"),
        QStringLiteral("electronics"), QStringLiteral("finance"),
        QStringLiteral("geography"), QStringLiteral("geology"),
        QStringLiteral("hamradio"), QStringLiteral("literature"),
        QStringLiteral("math"), QStringLiteral("medicalsoftware"),
        QStringLiteral("numericalanalysis"), QStringLiteral("physics"),
        QStringLiteral("presentation"), QStringLiteral("printing"),
        QStringLiteral("projectmanagement"), QStringLiteral("publishing"),
        QStringLiteral("robotics"), QStringLiteral("scanning"),
        QStringLiteral("science"), QStringLiteral("spreadsheet"),
        QStringLiteral("texteditor"), QStringLiteral("texttools"),
        QStringLiteral("wordprocessor"), QStringLiteral("x-mandrivalinux-office-other"),
        QStringLiteral("x-suse-core-office"), QStringLiteral("x-turbolinux-office"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryOffice), qPrintable(QStringLiteral("Missing Office for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllSystemEntries()
{
    const QStringList keys = {
        QStringLiteral("system"), QStringLiteral("accessibility"),
        QStringLiteral("advancedsettings"), QStringLiteral("archiving"),
        QStringLiteral("calculator"), QStringLiteral("calendar"),
        QStringLiteral("clock"), QStringLiteral("compiz"),
        QStringLiteral("compression"), QStringLiteral("desktopsettings"),
        QStringLiteral("desktoputility"), QStringLiteral("dialup"),
        QStringLiteral("discburning"), QStringLiteral("engineering"),
        QStringLiteral("filemanager"), QStringLiteral("filesystem"),
        QStringLiteral("filetools"), QStringLiteral("gnome"),
        QStringLiteral("hardwaresettings"), QStringLiteral("kde"),
        QStringLiteral("monitor"), QStringLiteral("packagemanager"),
        QStringLiteral("panel"), QStringLiteral("pda"),
        QStringLiteral("remoteaccess"), QStringLiteral("settings"),
        QStringLiteral("security"), QStringLiteral("systemsettings"),
        QStringLiteral("telephony"), QStringLiteral("telephonytools"),
        QStringLiteral("terminalemulator"), QStringLiteral("trayicon"),
        QStringLiteral("utility"), QStringLiteral("x-bluetooth"),
        QStringLiteral("x-digital_processing"), QStringLiteral("x-enlightenment"),
        QStringLiteral("x-gnome-networksettings"), QStringLiteral("x-gnome-personalsettings"),
        QStringLiteral("x-gnome-settings-panel"), QStringLiteral("x-gnome-systemsettings"),
        QStringLiteral("x-gnustep"), QStringLiteral("x-kde-internet"),
        QStringLiteral("x-kde-more"), QStringLiteral("x-kde-utilities-desktop"),
        QStringLiteral("x-kde-utilities-file"), QStringLiteral("x-kde-utilities-peripherals"),
        QStringLiteral("x-kde-utilities-pim"), QStringLiteral("x-lxde-settings"),
        QStringLiteral("x-mandrivalinux-internet-other"),
        QStringLiteral("x-mandrivalinux-system-archiving-backup"),
        QStringLiteral("x-misc"), QStringLiteral("x-novell-main"),
        QStringLiteral("x-red-hat-base"), QStringLiteral("x-red-hat-base-only"),
        QStringLiteral("x-red-hat-extra"), QStringLiteral("x-red-hat-serverconfig"),
        QStringLiteral("x-sun-supported"), QStringLiteral("x-suse-backup"),
        QStringLiteral("x-suse-controlcenter-lookandfeel"),
        QStringLiteral("x-suse-controlcenter-system"),
        QStringLiteral("x-suse-core"), QStringLiteral("x-suse-yast"),
        QStringLiteral("x-suse-yast-high_availability"),
        QStringLiteral("x-synthesis"), QStringLiteral("x-xfce"),
        QStringLiteral("x-xfce-toplevel"), QStringLiteral("x-xfcesettingsdialog"),
        QStringLiteral("x-ximian-main"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategorySystem), qPrintable(QStringLiteral("Missing System for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllInternetEntries()
{
    const QStringList keys = {
        QStringLiteral("internet"), QStringLiteral("email"),
        QStringLiteral("filetransfer"), QStringLiteral("network"),
        QStringLiteral("p2p"), QStringLiteral("videoconference"),
        QStringLiteral("webbrowser"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryInternet), qPrintable(QStringLiteral("Missing Internet for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllChatEntries()
{
    const QStringList keys = {
        QStringLiteral("chat"), QStringLiteral("contactmanagement"),
        QStringLiteral("instantmessaging"), QStringLiteral("ircclient"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryChat), qPrintable(QStringLiteral("Missing Chat for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllMusicEntries()
{
    const QStringList keys = {
        QStringLiteral("music"), QStringLiteral("audio"),
        QStringLiteral("audiovideo"), QStringLiteral("audiovideoediting"),
        QStringLiteral("cd"), QStringLiteral("midi"),
        QStringLiteral("mixer"), QStringLiteral("player"),
        QStringLiteral("recorder"), QStringLiteral("sequencer"),
        QStringLiteral("tuner"), QStringLiteral("x-alsa"),
        QStringLiteral("x-jack"), QStringLiteral("x-midi"),
        QStringLiteral("x-multitrack"), QStringLiteral("x-sequencers"),
        QStringLiteral("x-sound"), QStringLiteral("x-suse-sequencer"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryMusic), qPrintable(QStringLiteral("Missing Music for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllVideoEntries()
{
    const QStringList keys = {
        QStringLiteral("video"), QStringLiteral("audiovideo"),
        QStringLiteral("audiovideoediting"), QStringLiteral("multimedia"),
        QStringLiteral("player"), QStringLiteral("recorder"),
        QStringLiteral("tv"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryVideo), qPrintable(QStringLiteral("Missing Video for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllReadingEntries()
{
    // Known defect (xname2ty missing "reading" mapping): The XDG Desktop Menu
    // Specification registers "reading" as a main category, and the DDE-side
    // name2ty mapping includes it. However, the XDG-side xname2ty mapping
    // table in categoryutils.cpp is missing the "reading" -> Category::Reading
    // entry, so parseXdgCategoryString("reading") returns an empty list.
    // We assert the *correct* expected behavior and mark it QEXPECT_FAIL so
    // the test suite stays green while documenting the defect. Once DDE dev
    // team adds the missing mapping, the QEXPECT_FAIL should be removed.
    {
        const auto readingResult = CategoryUtils::parseXdgCategoryString(QStringLiteral("reading"));
        // QEXPECT_FAIL: xname2ty lacks "reading" -> Category::Reading entry;
        // parseXdgCategoryString("reading") currently returns empty.待 DDE 开发小队修复。
        QEXPECT_FAIL("", "xname2ty mapping missing 'reading' -> Category::Reading (known defect, 待 DDE 开发小队修复)", Continue);
        QVERIFY2(readingResult.contains(C::CategoryReading), "Expected Reading category for 'reading'");
    }

    const QStringList keys = {
        QStringLiteral("news"),
        QStringLiteral("translation"), QStringLiteral("x-bible"),
        QStringLiteral("x-islamic-software"), QStringLiteral("x-kde-edu-misc"),
        QStringLiteral("x-quran"), QStringLiteral("x-religion"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryReading), qPrintable(QStringLiteral("Missing Reading for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllDevelopmentEntries()
{
    const QStringList keys = {
        QStringLiteral("development"), QStringLiteral("building"),
        QStringLiteral("debugger"), QStringLiteral("guidesigner"),
        QStringLiteral("ide"), QStringLiteral("profiling"),
        QStringLiteral("revisioncontrol"), QStringLiteral("transiation"),
        QStringLiteral("webdevelopment"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryDevelopment), qPrintable(QStringLiteral("Missing Development for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringAllOthersEntries()
{
    const QStringList keys = {
        QStringLiteral("accessories"), QStringLiteral("applet"),
        QStringLiteral("consoleonly"), QStringLiteral("core"),
        QStringLiteral("editors"), QStringLiteral("favorites"),
        QStringLiteral("geoscience"), QStringLiteral("gpe"),
        QStringLiteral("meteorology"), QStringLiteral("motif"),
        QStringLiteral("pim"), QStringLiteral("playonlinux"),
        QStringLiteral("screensaver"), QStringLiteral("technical"),
        QStringLiteral("wine"), QStringLiteral("wine-programs-accessories"),
        QStringLiteral("x-mandriva-office-publishing"),
    };
    for (const auto &key : keys) {
        const auto result = CategoryUtils::parseXdgCategoryString(key);
        QVERIFY2(result.size() >= 1, qPrintable(QStringLiteral("No result for: ") + key));
        QVERIFY2(result.contains(C::CategoryOthers), qPrintable(QStringLiteral("Missing Others for: ") + key));
    }
}

void TestCategoryUtils::parseXdgCategoryStringMultipleDistinctCategories()
{
    QCOMPARE(CategoryUtils::parseXdgCategoryString(QStringLiteral("ide")).size(), 1);
    QVERIFY(CategoryUtils::parseXdgCategoryString(QStringLiteral("ide")).contains(C::CategoryDevelopment));
    QCOMPARE(CategoryUtils::parseXdgCategoryString(QStringLiteral("webdevelopment")).size(), 1);
    QVERIFY(CategoryUtils::parseXdgCategoryString(QStringLiteral("webdevelopment")).contains(C::CategoryDevelopment));
    QCOMPARE(CategoryUtils::parseXdgCategoryString(QStringLiteral("arcadegame")).size(), 1);
    QVERIFY(CategoryUtils::parseXdgCategoryString(QStringLiteral("arcadegame")).contains(C::CategoryGame));
    QCOMPARE(CategoryUtils::parseXdgCategoryString(QStringLiteral("wordprocessor")).size(), 1);
    QVERIFY(CategoryUtils::parseXdgCategoryString(QStringLiteral("wordprocessor")).contains(C::CategoryOffice));
    QCOMPARE(CategoryUtils::parseXdgCategoryString(QStringLiteral("ircclient")).size(), 1);
    QVERIFY(CategoryUtils::parseXdgCategoryString(QStringLiteral("ircclient")).contains(C::CategoryChat));
    QCOMPARE(CategoryUtils::parseXdgCategoryString(QStringLiteral("news")).size(), 1);
    QVERIFY(CategoryUtils::parseXdgCategoryString(QStringLiteral("news")).contains(C::CategoryReading));
}

// ---------------------------------------------------------------------------
// parseBestMatchedCategory
// ---------------------------------------------------------------------------

void TestCategoryUtils::bestMatchedSingleCategory()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("music")})), int(C::CategoryMusic));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("network")})), int(C::CategoryInternet));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("spreadsheet")})), int(C::CategoryOffice));
}

void TestCategoryUtils::bestMatchedEmptyReturnsOthers()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({})), int(C::CategoryOthers));
}

void TestCategoryUtils::bestMatchedOthersOnlyReturnsOthers()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("others")})), int(C::CategoryOthers));
}

void TestCategoryUtils::bestMatchedMusicVideoTiePrefersVideo()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("music"), QStringLiteral("video")})),
              int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedPlayerMapsToVideo()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("player")})), int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedMaxCountWins()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("game"), QStringLiteral("game"), QStringLiteral("music")})),
              int(C::CategoryGame));
}

void TestCategoryUtils::bestMatchedMultipleInternet()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("webbrowser"), QStringLiteral("email")})),
              int(C::CategoryInternet));
}

void TestCategoryUtils::bestMatchedDDECategoryWinsOverXdg()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("music"), QStringLiteral("audio")})),
              int(C::CategoryMusic));
}

void TestCategoryUtils::bestMatchedGameVariants()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("actiongame"), QStringLiteral("arcadegame")})),
              int(C::CategoryGame));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("boardgame"), QStringLiteral("cardgame"), QStringLiteral("puzzlegame")})),
              int(C::CategoryGame));
}

void TestCategoryUtils::bestMatchedDevelopmentVariants()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("ide"), QStringLiteral("debugger")})),
              int(C::CategoryDevelopment));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("building"), QStringLiteral("revisioncontrol")})),
              int(C::CategoryDevelopment));
}

void TestCategoryUtils::bestMatchedReadingVariants()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("news"), QStringLiteral("translation")})),
              int(C::CategoryReading));
}

void TestCategoryUtils::bestMatchedSystemVariants()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("desktopsettings"), QStringLiteral("packagemanager")})),
              int(C::CategorySystem));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("terminalemulator"), QStringLiteral("filemanager")})),
              int(C::CategorySystem));
}

void TestCategoryUtils::bestMatchedGraphicsVariants()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("2dgraphics"), QStringLiteral("rastergraphics")})),
              int(C::CategoryGraphics));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("photography"), QStringLiteral("viewer")})),
              int(C::CategoryGraphics));
}

void TestCategoryUtils::bestMatchedChatVariants()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("instantmessaging"), QStringLiteral("contactmanagement")})),
              int(C::CategoryChat));
}

void TestCategoryUtils::bestMatchedOfficeVariants()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("spreadsheet"), QStringLiteral("presentation")})),
              int(C::CategoryOffice));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("science"), QStringLiteral("math")})),
              int(C::CategoryOffice));
}

void TestCategoryUtils::bestMatchedMixedUnknownAndKnown()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("totally-unknown"), QStringLiteral("music")})),
              int(C::CategoryMusic));
}

void TestCategoryUtils::bestMatchedAudioVideoEditingTie()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("audiovideoediting")})),
              int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedRecorderTie()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("recorder")})),
              int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedNonMusicVideoTieReturnsSortedFirst()
{
    // Tie between Game (enum 5) and Office (enum 6) — no Music/Video involved.
    // After sort, Game < Office, so maxCatogories[0] = Game.
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("game"), QStringLiteral("office")})),
              int(C::CategoryGame));
    // Tie between Internet (enum 0) and Chat (enum 1)
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("internet"), QStringLiteral("chat")})),
              int(C::CategoryInternet));
    // Tie between Reading (enum 7) and Development (enum 8)
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("reading"), QStringLiteral("development")})),
              int(C::CategoryReading));
}

void TestCategoryUtils::bestMatchedAccessoriesRemovedReturnsOthers()
{
    // "accessories" maps to CategoryOthers via XDG, which gets removed.
    // After removal the map is empty → returns CategoryOthers.
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("accessories")})),
              int(C::CategoryOthers));
    // Mix of accessories (Others, removed) and unknown → empty map → Others
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("accessories"), QStringLiteral("totally-unknown")})),
              int(C::CategoryOthers));
}

void TestCategoryUtils::bestMatchedAllUnknownReturnsOthers()
{
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("foo"), QStringLiteral("bar"), QStringLiteral("baz")})),
              int(C::CategoryOthers));
}

void TestCategoryUtils::bestMatchedDDEAndXdgSameTypeAccumulates()
{
    // "game" is a DDE name → CategoryGame (count 1)
    // "actiongame" is an XDG name → CategoryGame (count 1)
    // Total Game count = 2, should beat a single Office from "spreadsheet"
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("game"), QStringLiteral("actiongame"), QStringLiteral("spreadsheet")})),
              int(C::CategoryGame));
}

void TestCategoryUtils::bestMatchedThreeWayTie()
{
    // Three-way tie among Internet, Music, Video — Music+Video present → Video
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("internet"), QStringLiteral("music"), QStringLiteral("video")})),
              int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedCaseInsensitive()
{
    // parseBestMatchedCategory lowercases each category before lookup
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("MUSIC")})),
              int(C::CategoryMusic));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("Game"), QStringLiteral("GAME")})),
              int(C::CategoryGame));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("WEBBROWSER")})),
              int(C::CategoryInternet));
}

QTEST_MAIN(TestCategoryUtils)
#include "categoryutilstest.moc"
