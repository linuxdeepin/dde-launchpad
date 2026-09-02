// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QLoggingCategory>

#include "../src/utils/categoryutils.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

using C = CategoryUtils::Categorytype;
}

class TestCategoryUtils : public QObject
{
    Q_OBJECT
private slots:
    void parseDDECategoryStringKnown();
    void parseDDECategoryStringUnknown();
    void parseXdgCategoryStringSingle();
    void parseXdgCategoryStringMultiValued();
    void parseXdgCategoryStringUnknown();
    void bestMatchedSingleCategory();
    void bestMatchedEmptyReturnsOthers();
    void bestMatchedOthersOnlyReturnsOthers();
    void bestMatchedMusicVideoTiePrefersVideo();
    void bestMatchedPlayerMapsToVideo();
    void bestMatchedMaxCountWins();
    void bestMatchedMultipleInternet();
    void parseDDECategoryStringAllKnown();
    void parseXdgCategoryStringMultipleDistinctCategories();
    void parseXdgCategoryStringAudioMapsToMusic();
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
};

void TestCategoryUtils::parseDDECategoryStringKnown()
{
    qCInfo(logTest) << "parseDDECategoryString should map the known DDE category names";
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("music"))), int(C::CategoryMusic));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("internet"))), int(C::CategoryInternet));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("development"))), int(C::CategoryDevelopment));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("others"))), int(C::CategoryOthers));
}

void TestCategoryUtils::parseDDECategoryStringUnknown()
{
    qCInfo(logTest) << "Unknown DDE category strings should yield CategoryErr";
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("unknown"))), int(C::CategoryErr));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("Music"))), int(C::CategoryErr)); // case sensitive
}

void TestCategoryUtils::parseXdgCategoryStringSingle()
{
    qCInfo(logTest) << "parseXdgCategoryString should map single-valued XDG categories";
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("webbrowser"));
    QCOMPARE(result.size(), 1);
    QVERIFY(result.contains(C::CategoryInternet));
}

void TestCategoryUtils::parseXdgCategoryStringMultiValued()
{
    qCInfo(logTest) << "audiovideo should map to both music and video";
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("audiovideo"));
    QCOMPARE(result.size(), 2);
    QVERIFY(result.contains(C::CategoryMusic));
    QVERIFY(result.contains(C::CategoryVideo));
}

void TestCategoryUtils::parseXdgCategoryStringUnknown()
{
    qCInfo(logTest) << "Unknown XDG category strings should yield an empty list";
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("totally-unknown-category"));
    QVERIFY(result.isEmpty());
}

void TestCategoryUtils::bestMatchedSingleCategory()
{
    qCInfo(logTest) << "A single matching category should win";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("music")})), int(C::CategoryMusic));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("network")})), int(C::CategoryInternet));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("spreadsheet")})), int(C::CategoryOffice));
}

void TestCategoryUtils::bestMatchedEmptyReturnsOthers()
{
    qCInfo(logTest) << "An empty category list should be classified as others";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({})), int(C::CategoryOthers));
}

void TestCategoryUtils::bestMatchedOthersOnlyReturnsOthers()
{
    qCInfo(logTest) << "An only-others list should be classified as others";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("others")})), int(C::CategoryOthers));
}

void TestCategoryUtils::bestMatchedMusicVideoTiePrefersVideo()
{
    qCInfo(logTest) << "A music/video tie should prefer video";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("music"), QStringLiteral("video")})),
              int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedPlayerMapsToVideo()
{
    qCInfo(logTest) << "player (maps to music+video) should resolve to video";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("player")})), int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedMaxCountWins()
{
    qCInfo(logTest) << "The category with the highest count should win";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("game"), QStringLiteral("game"), QStringLiteral("music")})),
              int(C::CategoryGame));
}

void TestCategoryUtils::bestMatchedMultipleInternet()
{
    qCInfo(logTest) << "Several internet categories should aggregate into internet";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("webbrowser"), QStringLiteral("email")})),
              int(C::CategoryInternet));
}

void TestCategoryUtils::parseDDECategoryStringAllKnown()
{
    qCInfo(logTest) << "parseDDECategoryString should map all known DDE category names";
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("chat"))), int(C::CategoryChat));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("video"))), int(C::CategoryVideo));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("graphics"))), int(C::CategoryGraphics));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("office"))), int(C::CategoryOffice));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("game"))), int(C::CategoryGame));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("reading"))), int(C::CategoryReading));
    QCOMPARE(int(CategoryUtils::parseDDECategoryString(QStringLiteral("system"))), int(C::CategorySystem));
}

void TestCategoryUtils::parseXdgCategoryStringMultipleDistinctCategories()
{
    qCInfo(logTest) << "parseXdgCategoryString for several distinct known categories";
    // Each call returns the list for that single string; verify a batch of them
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

void TestCategoryUtils::parseXdgCategoryStringAudioMapsToMusic()
{
    qCInfo(logTest) << "audio should map to music";
    const auto result = CategoryUtils::parseXdgCategoryString(QStringLiteral("audio"));
    QCOMPARE(result.size(), 1);
    QVERIFY(result.contains(C::CategoryMusic));
}

void TestCategoryUtils::bestMatchedDDECategoryWinsOverXdg()
{
    qCInfo(logTest) << "A DDE category name should be matched before falling to XDG parsing";
    // 'music' is a DDE name -> CategoryMusic (via parseDDECategoryString)
    // 'audio' is an XDG name -> CategoryMusic (via parseXdgCategoryString)
    // mixing them should still yield Music
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("music"), QStringLiteral("audio")})),
              int(C::CategoryMusic));
}

void TestCategoryUtils::bestMatchedGameVariants()
{
    qCInfo(logTest) << "Various game XDG categories should aggregate into game";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("actiongame"), QStringLiteral("arcadegame")})),
              int(C::CategoryGame));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("boardgame"), QStringLiteral("cardgame"), QStringLiteral("puzzlegame")})),
              int(C::CategoryGame));
}

void TestCategoryUtils::bestMatchedDevelopmentVariants()
{
    qCInfo(logTest) << "Development XDG categories should aggregate into development";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("ide"), QStringLiteral("debugger")})),
              int(C::CategoryDevelopment));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("building"), QStringLiteral("revisioncontrol")})),
              int(C::CategoryDevelopment));
}

void TestCategoryUtils::bestMatchedReadingVariants()
{
    qCInfo(logTest) << "Reading XDG categories should aggregate into reading";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("news"), QStringLiteral("translation")})),
              int(C::CategoryReading));
}

void TestCategoryUtils::bestMatchedSystemVariants()
{
    qCInfo(logTest) << "System XDG categories should aggregate into system";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("desktopsettings"), QStringLiteral("packagemanager")})),
              int(C::CategorySystem));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("terminalemulator"), QStringLiteral("filemanager")})),
              int(C::CategorySystem));
}

void TestCategoryUtils::bestMatchedGraphicsVariants()
{
    qCInfo(logTest) << "Graphics XDG categories should aggregate into graphics";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("2dgraphics"), QStringLiteral("rastergraphics")})),
              int(C::CategoryGraphics));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("photography"), QStringLiteral("viewer")})),
              int(C::CategoryGraphics));
}

void TestCategoryUtils::bestMatchedChatVariants()
{
    qCInfo(logTest) << "Chat XDG categories should aggregate into chat";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("instantmessaging"), QStringLiteral("contactmanagement")})),
              int(C::CategoryChat));
}

void TestCategoryUtils::bestMatchedOfficeVariants()
{
    qCInfo(logTest) << "Office XDG categories should aggregate into office";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("spreadsheet"), QStringLiteral("presentation")})),
              int(C::CategoryOffice));
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("science"), QStringLiteral("math")})),
              int(C::CategoryOffice));
}

void TestCategoryUtils::bestMatchedMixedUnknownAndKnown()
{
    qCInfo(logTest) << "Unknown categories mixed with known ones should ignore unknowns";
    // 'totally-unknown' yields CategoryErr from DDE and empty from XDG -> ignored
    // 'music' yields CategoryMusic -> should win
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("totally-unknown"), QStringLiteral("music")})),
              int(C::CategoryMusic));
}

void TestCategoryUtils::bestMatchedAudioVideoEditingTie()
{
    qCInfo(logTest) << "audiovideoediting maps to both music and video -> tie -> video";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("audiovideoediting")})),
              int(C::CategoryVideo));
}

void TestCategoryUtils::bestMatchedRecorderTie()
{
    qCInfo(logTest) << "recorder maps to both music and video -> tie -> video";
    QCOMPARE(int(CategoryUtils::parseBestMatchedCategory({QStringLiteral("recorder")})),
              int(C::CategoryVideo));
}

QTEST_MAIN(TestCategoryUtils)
#include "categoryutilstest.moc"
