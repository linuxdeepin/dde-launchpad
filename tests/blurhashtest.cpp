// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QLoggingCategory>

#include "../src/utils/blurhash.hpp"

#include <vector>

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

std::vector<unsigned char> solidImage(size_t width, size_t height,
                                      unsigned char r, unsigned char g, unsigned char b)
{
    std::vector<unsigned char> image(width * height * 3, 0);
    for (size_t i = 0; i < width * height; ++i) {
        image[i * 3 + 0] = r;
        image[i * 3 + 1] = g;
        image[i * 3 + 2] = b;
    }
    return image;
}

// blurhash layout: 1 (components) + 1 (maxAC) + 4 (DC) + 2 bytes per AC component
size_t expectedHashLength(int cx, int cy)
{
    return size_t(1 + 1 + 4 + (cx * cy - 1) * 2);
}
}

class TestBlurhash : public QObject
{
    Q_OBJECT
private slots:
    void decodeEmptyStringReturnsEmpty();
    void decodeInvalidHashReturnsEmpty();
    void encodeRejectsInvalidInput();
    void encodedHashHasExpectedLength();
    void decodedImageHasExpectedDimensions();
    void roundTripPreservesAverageColor();
    void decodeWithFourBytesPerPixel();
    void decodeTooShortHashReturnsEmpty();
    void decodeWrongSizeForComponentsReturnsEmpty();
};

void TestBlurhash::decodeEmptyStringReturnsEmpty()
{
    qCInfo(logTest) << "Decoding an empty hash should return an empty image";
    const auto img = blurhash::decode(std::string_view{}, 8, 8);
    QVERIFY(img.image.empty());
    QCOMPARE(img.width, size_t(0));
    QCOMPARE(img.height, size_t(0));
}

void TestBlurhash::decodeInvalidHashReturnsEmpty()
{
    qCInfo(logTest) << "Decoding a hash with invalid characters should return an empty image";
    const auto img = blurhash::decode(std::string_view("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"), 8, 8);
    QVERIFY(img.image.empty());
}

void TestBlurhash::encodeRejectsInvalidInput()
{
    qCInfo(logTest) << "encode should return an empty string for invalid parameters";
    std::vector<unsigned char> image = solidImage(4, 4, 255, 0, 0);
    QVERIFY(blurhash::encode(image.data(), 0, 4, 1, 1).empty());   // width 0
    QVERIFY(blurhash::encode(nullptr, 4, 4, 1, 1).empty());        // null image
    QVERIFY(blurhash::encode(image.data(), 4, 4, 0, 1).empty());   // components_x < 1
    QVERIFY(blurhash::encode(image.data(), 4, 4, 1, 10).empty());  // components_y > 9
}

void TestBlurhash::encodedHashHasExpectedLength()
{
    qCInfo(logTest) << "encode should produce a hash of the documented length";
    std::vector<unsigned char> image = solidImage(8, 8, 128, 64, 200);
    QCOMPARE(blurhash::encode(image.data(), 8, 8, 1, 1).size(), size_t(6));                // no AC components
    QCOMPARE(blurhash::encode(image.data(), 8, 8, 1, 3).size(), expectedHashLength(1, 3)); // 10
    QCOMPARE(blurhash::encode(image.data(), 8, 8, 2, 2).size(), expectedHashLength(2, 2)); // 12
    QCOMPARE(blurhash::encode(image.data(), 8, 8, 4, 4).size(), expectedHashLength(4, 4)); // 36
}

void TestBlurhash::decodedImageHasExpectedDimensions()
{
    qCInfo(logTest) << "decode should produce an image matching the requested dimensions";
    constexpr size_t width = 16, height = 16;
    std::vector<unsigned char> image = solidImage(width, height, 128, 64, 200);
    const std::string hash = blurhash::encode(image.data(), width, height, 2, 2);
    QVERIFY(!hash.empty());

    const auto decoded = blurhash::decode(hash, width, height);
    QCOMPARE(decoded.width, width);
    QCOMPARE(decoded.height, height);
    QCOMPARE(decoded.image.size(), width * height * 3);
}

void TestBlurhash::roundTripPreservesAverageColor()
{
    qCInfo(logTest) << "Round-trip should preserve the average color (DC component)";
    constexpr size_t width = 16, height = 16;
    const unsigned char r = 128, g = 64, b = 200;
    std::vector<unsigned char> image = solidImage(width, height, r, g, b);

    const std::string hash = blurhash::encode(image.data(), width, height, 3, 3);
    QVERIFY(!hash.empty());

    const auto decoded = blurhash::decode(hash, width, height);
    QCOMPARE(decoded.image.size(), width * height * 3);

    long rs = 0, gs = 0, bs = 0;
    for (size_t i = 0; i < width * height; ++i) {
        rs += decoded.image[i * 3 + 0];
        gs += decoded.image[i * 3 + 1];
        bs += decoded.image[i * 3 + 2];
    }
    const long n = long(width * height);
    const long avgR = rs / n, avgG = gs / n, avgB = bs / n;
    // blurhash is lossy per-pixel, but the average (DC) stays close to the source color
    QVERIFY(avgR >= long(r) - 15 && avgR <= long(r) + 15);
    QVERIFY(avgG >= long(g) - 15 && avgG <= long(g) + 15);
    QVERIFY(avgB >= long(b) - 15 && avgB <= long(b) + 15);
}

void TestBlurhash::decodeWithFourBytesPerPixel()
{
    qCInfo(logTest) << "decode with bytesPerPixel=4 should produce a larger buffer";
    constexpr size_t width = 16, height = 16;
    std::vector<unsigned char> image = solidImage(width, height, 128, 64, 200);
    const std::string hash = blurhash::encode(image.data(), width, height, 3, 3);
    QVERIFY(!hash.empty());

    const auto decoded = blurhash::decode(hash, width, height, 4);
    QCOMPARE(decoded.width, width);
    QCOMPARE(decoded.height, height);
    // buffer should be width * height * 4 (not 3)
    QCOMPARE(decoded.image.size(), width * height * 4);
    // first 3 bytes of each pixel should still be the color channels
    QVERIFY(decoded.image[0] != 255 || decoded.image[1] != 255 || decoded.image[2] != 255);
}

void TestBlurhash::decodeTooShortHashReturnsEmpty()
{
    qCInfo(logTest) << "decode with a hash shorter than 10 chars should return empty";
    // hash must be >= 10 chars (1 + 1 + 4 + at least 4 for 2x2 components - 1 AC = 2 bytes -> 8, but 10 is the minimum check)
    const auto img = blurhash::decode(std::string_view("short"), 8, 8);
    QVERIFY(img.image.empty());
}

void TestBlurhash::decodeWrongSizeForComponentsReturnsEmpty()
{
    qCInfo(logTest) << "decode with a hash whose length doesn't match the component count should return empty";
    // 1 component char says 4x4 = 16 components -> needs 1+1+4+(16-1)*2 = 36 chars
    // but we'll give only 12 chars
    const auto img = blurhash::decode(std::string_view("LFE.}?a]a]a]a]a]a]a]"), 8, 8);
    // this hash has wrong length for its declared components -> should return empty
    QVERIFY(img.image.empty());
}

QTEST_MAIN(TestBlurhash)
#include "blurhashtest.moc"
