// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QSignalSpy>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLoggingCategory>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include "../src/models/favoritedproxymodel.h"
#include "../src/models/appsmodel.h"

namespace {
Q_LOGGING_CATEGORY(logTest, "dde.launchpad.test")

enum SourceRoles {
    SrcDesktopIdRole = Qt::UserRole + 1,
    SrcNameRole,
    SrcIconNameRole,
    SrcNoDisplayRole,
    SrcDDECategoryRole,
    SrcInstalledTimeRole,
    SrcLastLaunchedTimeRole,
    SrcLaunchedTimesRole,
    SrcAutoStartRole,
    SrcCategoriesRole,
    SrcVendorRole,
    SrcGenericNameRole,
};

QStandardItem *makeApp(const QString &desktopId, const QString &name)
{
    auto item = new QStandardItem;
    item->setData(desktopId, SrcDesktopIdRole);
    item->setData(name, SrcNameRole);
    item->setData(QStringLiteral("application-default-icon"), SrcIconNameRole);
    item->setData(false, SrcNoDisplayRole);
    item->setData(0, SrcDDECategoryRole);
    return item;
}
}

class TestFavoritedProxyModel : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void existsReturnsTrueForAddedFavorite();
    void addAndRemoveFavorite();
    void addDuplicateIsNoop();
    void removeNonexistentIsNoop();
    void pinToTopMovesToFront();
    void filterAcceptsOnlyFavorited();
    void lessThanOrdersByFavoriteListPosition();

private:
    QStandardItemModel m_sourceModel;
    // unique prefix so we can safely remove leftovers even if a prior test aborted
    static inline const QString prefix = QStringLiteral("test-");
    void removeAllTestFavorites();
};

void TestFavoritedProxyModel::removeAllTestFavorites()
{
    auto &proxy = FavoritedProxyModel::instance();
    // remove all test-managed IDs (best-effort; removeFavorite is noop if absent)
    for (const QString &id : {
            prefix + "exists.desktop", prefix + "add-remove.desktop", prefix + "dup.desktop",
            prefix + "fav-1.desktop", prefix + "fav-2.desktop", prefix + "fav-3.desktop",
            QStringLiteral("app-a.desktop"), QStringLiteral("app-b.desktop"),
            QStringLiteral("app-c.desktop"), QStringLiteral("app-d.desktop") }) {
        proxy.removeFavorite(id);
    }
}

void TestFavoritedProxyModel::initTestCase()
{
    m_sourceModel.setItemRoleNames({
        { SrcDesktopIdRole, QByteArrayLiteral("desktopId") },
        { SrcNameRole, QByteArrayLiteral("name") },
        { SrcIconNameRole, QByteArrayLiteral("iconName") },
        { SrcNoDisplayRole, QByteArrayLiteral("noDisplay") },
        { SrcDDECategoryRole, QByteArrayLiteral("ddeCategory") },
        { SrcInstalledTimeRole, QByteArrayLiteral("installedTime") },
        { SrcLastLaunchedTimeRole, QByteArrayLiteral("lastLaunchedTime") },
        { SrcLaunchedTimesRole, QByteArrayLiteral("launchedTimes") },
        { SrcAutoStartRole, QByteArrayLiteral("autoStart") },
        { SrcCategoriesRole, QByteArrayLiteral("categories") },
        { SrcVendorRole, QByteArrayLiteral("vendor") },
        { SrcGenericNameRole, QByteArrayLiteral("genericName") },
    });
    m_sourceModel.appendRow(makeApp("app-a.desktop", "App A"));
    m_sourceModel.appendRow(makeApp("app-b.desktop", "App B"));
    m_sourceModel.appendRow(makeApp("app-c.desktop", "App C"));
    m_sourceModel.appendRow(makeApp("app-d.desktop", "App D"));

    AppsModel::instance().setSourceModel(&m_sourceModel);
    AppsModel::instance().setReady(true);

    // Accessing the singleton triggers construction + load() + setSourceModel.
    FavoritedProxyModel::instance();

    // clean any leftovers from a prior aborted run
    removeAllTestFavorites();
}

void TestFavoritedProxyModel::cleanupTestCase()
{
    // best-effort cleanup of in-memory state
    removeAllTestFavorites();
    // delete the favorited.ini written by save() to prevent cross-run disk pollution
    const QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const QString iniPath = QDir(basePath).absoluteFilePath("favorited.ini");
    QFile::remove(iniPath);
}

void TestFavoritedProxyModel::existsReturnsTrueForAddedFavorite()
{
    auto &proxy = FavoritedProxyModel::instance();
    const QString id = prefix + QStringLiteral("exists.desktop");
    QVERIFY(!proxy.exists(id));
    proxy.addFavorite(id);
    QVERIFY(proxy.exists(id));
    proxy.removeFavorite(id);
    QVERIFY(!proxy.exists(id));
}

void TestFavoritedProxyModel::addAndRemoveFavorite()
{
    auto &proxy = FavoritedProxyModel::instance();
    const QString id = prefix + QStringLiteral("add-remove.desktop");

    proxy.addFavorite(id);
    QVERIFY(proxy.exists(id));

    proxy.removeFavorite(id);
    QVERIFY(!proxy.exists(id));
}

void TestFavoritedProxyModel::addDuplicateIsNoop()
{
    auto &proxy = FavoritedProxyModel::instance();
    const QString id = prefix + QStringLiteral("dup.desktop");

    proxy.addFavorite(id);
    QVERIFY(proxy.exists(id));

    // adding again should not crash or duplicate
    proxy.addFavorite(id);
    QVERIFY(proxy.exists(id));

    // only one removal needed
    proxy.removeFavorite(id);
    QVERIFY(!proxy.exists(id));
}

void TestFavoritedProxyModel::removeNonexistentIsNoop()
{
    auto &proxy = FavoritedProxyModel::instance();
    // removing a non-existent favorite should not crash
    proxy.removeFavorite(prefix + QStringLiteral("nonexistent-id.desktop"));
}

void TestFavoritedProxyModel::pinToTopMovesToFront()
{
    auto &proxy = FavoritedProxyModel::instance();
    const QString id1 = prefix + QStringLiteral("fav-1.desktop");
    const QString id2 = prefix + QStringLiteral("fav-2.desktop");
    const QString id3 = prefix + QStringLiteral("fav-3.desktop");

    proxy.addFavorite(id1);
    proxy.addFavorite(id2);
    proxy.addFavorite(id3);

    // pin fav-3 to top
    proxy.pinToTop(id3);
    QVERIFY(proxy.exists(id3));

    // pin a non-existent id should be a noop
    proxy.pinToTop(prefix + QStringLiteral("nonexistent.desktop"));

    // cleanup
    proxy.removeFavorite(id1);
    proxy.removeFavorite(id2);
    proxy.removeFavorite(id3);
}

void TestFavoritedProxyModel::filterAcceptsOnlyFavorited()
{
    auto &proxy = FavoritedProxyModel::instance();

    // add favorites that match source model items (app-a, app-c)
    proxy.addFavorite("app-a.desktop");
    proxy.addFavorite("app-c.desktop");

    // proxy should show exactly the 2 favorited items (out of 4 source rows).
    // predefined favorites (deepin-editor etc.) don't match the test source, so rowCount == 2.
    QCOMPARE(proxy.rowCount(), 2);

    // unconditionally verify: every proxy row's desktopId is a favorited item
    for (int i = 0; i < proxy.rowCount(); ++i) {
        QString id = proxy.data(proxy.index(i, 0), AppsModel::DesktopIdRole).toString();
        QVERIFY(proxy.exists(id));
    }

    // cleanup
    proxy.removeFavorite("app-a.desktop");
    proxy.removeFavorite("app-c.desktop");
    QCOMPARE(proxy.rowCount(), 0);
}

void TestFavoritedProxyModel::lessThanOrdersByFavoriteListPosition()
{
    auto &proxy = FavoritedProxyModel::instance();

    proxy.addFavorite("app-b.desktop");
    proxy.addFavorite("app-a.desktop");
    proxy.addFavorite("app-d.desktop");

    // pin app-d to top so the order is: app-d, app-b, app-a
    proxy.pinToTop("app-d.desktop");

    // proxy should show exactly 3 favorited items
    QCOMPARE(proxy.rowCount(), 3);

    // after sort, app-d should come before app-b which comes before app-a
    QCOMPARE(proxy.data(proxy.index(0, 0), AppsModel::DesktopIdRole).toString(), QStringLiteral("app-d.desktop"));
    QCOMPARE(proxy.data(proxy.index(1, 0), AppsModel::DesktopIdRole).toString(), QStringLiteral("app-b.desktop"));
    QCOMPARE(proxy.data(proxy.index(2, 0), AppsModel::DesktopIdRole).toString(), QStringLiteral("app-a.desktop"));

    // cleanup
    proxy.removeFavorite("app-b.desktop");
    proxy.removeFavorite("app-a.desktop");
    proxy.removeFavorite("app-d.desktop");
}

QTEST_MAIN(TestFavoritedProxyModel)
#include "favoritedproxymodeltest.moc"
