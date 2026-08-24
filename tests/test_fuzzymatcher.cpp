#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QStandardPaths>
#include "../src/fuzzymatcher.h"
#include "../src/appindexer.h"

class TestFuzzyMatcher : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Enforce test mode isolation so frecency/config files go to isolated temp path instead of ~/.config/quasar
        QStandardPaths::setTestModeEnabled(true);
        qDebug() << "Test mode enabled. Config isolation path:" 
                 << QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    }

    void testDamerauLevenshteinTransposition() {
        QStandardItemModel sourceModel;
        QHash<int, QByteArray> roles;
        roles[Qt::UserRole + 1] = "name";
        roles[Qt::UserRole + 2] = "exec";
        roles[Qt::UserRole + 3] = "desktopFile";
        sourceModel.setItemRoleNames(roles);
        
        QStandardItem *item1 = new QStandardItem();
        item1->setData("Steam", Qt::UserRole + 1);
        item1->setData("steam", Qt::UserRole + 2);
        item1->setData("steam.desktop", Qt::UserRole + 3);

        QStandardItem *item2 = new QStandardItem();
        item2->setData("Calculator", Qt::UserRole + 1);
        item2->setData("galculator", Qt::UserRole + 2);
        item2->setData("calculator.desktop", Qt::UserRole + 3);

        sourceModel.appendRow(item1);
        sourceModel.appendRow(item2);

        FuzzyMatcher matcher;
        matcher.setSourceModel(&sourceModel);

        // "stema" has 1 transposition edit distance from "Steam"
        matcher.setQuery("stema");
        QCOMPARE(matcher.rowCount() > 0, true);

        QModelIndex firstIdx = matcher.index(0, 0);
        QString matchedName = matcher.data(firstIdx, Qt::UserRole + 1).toString();
        QCOMPARE(matchedName, QString("Steam"));

        int score = matcher.data(firstIdx, FuzzyMatcher::ScoreRole).toInt();
        QCOMPARE(score > 0, true);
    }

    void testAcronymsAndTransliteration() {
        QStandardItemModel sourceModel;
        QHash<int, QByteArray> roles;
        roles[Qt::UserRole + 1] = "name";
        roles[Qt::UserRole + 2] = "exec";
        roles[Qt::UserRole + 3] = "desktopFile";
        sourceModel.setItemRoleNames(roles);

        QStandardItem *item1 = new QStandardItem();
        item1->setData(QString::fromUtf8("Привет"), Qt::UserRole + 1);
        item1->setData("hello", Qt::UserRole + 2);
        item1->setData("hello.desktop", Qt::UserRole + 3);

        QStandardItem *item2 = new QStandardItem();
        item2->setData("Firefox Web Browser", Qt::UserRole + 1);
        item2->setData("firefox", Qt::UserRole + 2);
        item2->setData("firefox.desktop", Qt::UserRole + 3);

        sourceModel.appendRow(item1);
        sourceModel.appendRow(item2);

        FuzzyMatcher matcher;
        matcher.setSourceModel(&sourceModel);

        // Transliteration query: "ghbdtn" -> "привет"
        matcher.setQuery("ghbdtn");
        QCOMPARE(matcher.rowCount() > 0, true);

        QModelIndex idx1 = matcher.index(0, 0);
        QString matchedName1 = matcher.data(idx1, Qt::UserRole + 1).toString();
        QCOMPARE(matchedName1, QString::fromUtf8("Привет"));

        // Acronym query: "fwb" -> "Firefox Web Browser"
        matcher.setQuery("fwb");
        QCOMPARE(matcher.rowCount() > 0, true);
        QModelIndex idx2 = matcher.index(0, 0);
        QString matchedName2 = matcher.data(idx2, Qt::UserRole + 1).toString();
        QCOMPARE(matchedName2, QString("Firefox Web Browser"));
    }

    void testWindowPrefixStripsScheme() {
        QStandardItemModel sourceModel;
        QHash<int, QByteArray> roles;
        roles[Qt::UserRole + 1] = "name";
        roles[Qt::UserRole + 2] = "exec";
        roles[Qt::UserRole + 3] = "desktopFile";
        sourceModel.setItemRoleNames(roles);

        QStandardItem *item1 = new QStandardItem();
        item1->setData("firefox — Mozilla Firefox", Qt::UserRole + 1);
        item1->setData("address:0x55f123", Qt::UserRole + 2);
        item1->setData("window:0x55f123", Qt::UserRole + 3);
        sourceModel.appendRow(item1);

        FuzzyMatcher matcher;
        matcher.setWindowModel(&sourceModel);
        matcher.setQuery("w:fire");

        QCOMPARE(matcher.rowCount(), 1);
        QModelIndex idx = matcher.index(0, 0);
        QString matchedName = matcher.data(idx, Qt::UserRole + 1).toString();
        QCOMPARE(matchedName, QString("firefox — Mozilla Firefox"));

        // Test w. prefix
        matcher.setQuery("w.fire");
        QCOMPARE(matcher.rowCount(), 1);
        QCOMPARE(matcher.data(matcher.index(0, 0), Qt::UserRole + 1).toString(), QString("firefox — Mozilla Firefox"));
    }

    void testNegativeMatches() {
        QStandardItemModel sourceModel;
        QHash<int, QByteArray> roles;
        roles[Qt::UserRole + 1] = "name";
        roles[Qt::UserRole + 2] = "exec";
        sourceModel.setItemRoleNames(roles);

        QStandardItem *item1 = new QStandardItem();
        item1->setData("Steam", Qt::UserRole + 1);
        item1->setData("steam", Qt::UserRole + 2);
        sourceModel.appendRow(item1);

        FuzzyMatcher matcher;
        matcher.setSourceModel(&sourceModel);

        // "xyz12399" should have DL distance > maxDist and produce no match
        matcher.setQuery("xyz12399");
        QCOMPARE(matcher.rowCount(), 0);
    }

    void testInlineCalculator() {
        QStandardItemModel sourceModel;
        QHash<int, QByteArray> roles;
        roles[Qt::UserRole + 1] = "name";
        roles[Qt::UserRole + 4] = "exec";
        sourceModel.setItemRoleNames(roles);

        FuzzyMatcher matcher;
        matcher.setAppIndexerModel(&sourceModel);

        // 125 * 8 = 1000
        matcher.setQuery("125 * 8");
        QCOMPARE(matcher.rowCount(), 1);
        QCOMPARE(matcher.data(matcher.index(0, 0), Qt::UserRole + 1).toString(), QString("= 1000"));
        QCOMPARE(matcher.data(matcher.index(0, 0), Qt::UserRole + 4).toString(), QString("__copy__:1000"));

        // (50 + 25) / 3 = 25
        matcher.setQuery("(50 + 25) / 3");
        QCOMPARE(matcher.rowCount(), 1);
        QCOMPARE(matcher.data(matcher.index(0, 0), Qt::UserRole + 1).toString(), QString("= 25"));

        // 2^8 = 256
        matcher.setQuery("2^8");
        QCOMPARE(matcher.rowCount(), 1);
        QCOMPARE(matcher.data(matcher.index(0, 0), Qt::UserRole + 1).toString(), QString("= 256"));
    }

    void testEmojiAndClipboardPrefixes() {
        QStandardItemModel emojiModel;
        QHash<int, QByteArray> emojiRoles;
        emojiRoles[Qt::UserRole + 1] = "name";
        emojiRoles[Qt::UserRole + 4] = "exec";
        emojiModel.setItemRoleNames(emojiRoles);

        QStandardItem *e1 = new QStandardItem();
        e1->setData("Fire / Огонь", Qt::UserRole + 1);
        e1->setData("__copy__:🔥", Qt::UserRole + 4);
        emojiModel.appendRow(e1);

        QStandardItemModel clipModel;
        QHash<int, QByteArray> clipRoles;
        clipRoles[Qt::UserRole + 1] = "name";
        clipRoles[Qt::UserRole + 4] = "exec";
        clipModel.setItemRoleNames(clipRoles);

        QStandardItem *c1 = new QStandardItem();
        c1->setData("echo 'hello world'", Qt::UserRole + 1);
        c1->setData("__clipboard__:0", Qt::UserRole + 4);
        clipModel.appendRow(c1);

        FuzzyMatcher matcher;
        matcher.setEmojiModel(&emojiModel);
        matcher.setClipboardModel(&clipModel);

        // Emoji query
        matcher.setQuery("e.fire");
        QCOMPARE(matcher.rowCount(), 1);
        QCOMPARE(matcher.data(matcher.index(0, 0), Qt::UserRole + 1).toString(), QString("Fire / Огонь"));

        // Clipboard query
        matcher.setQuery("c.hello");
        QCOMPARE(matcher.rowCount(), 1);
        QCOMPARE(matcher.data(matcher.index(0, 0), Qt::UserRole + 1).toString(), QString("echo 'hello world'"));
    }

    void testCalculatorSecurityRejections() {
        Calculator calc;
        // Non-whitelisted identifier access must be rejected
        QVERIFY(!calc.evaluate("constructor.constructor('return 1')()").has_value());
        QVERIFY(!calc.evaluate("eval('2+2')").has_value());
        QVERIFY(!calc.evaluate("this.toString()").has_value());
        QVERIFY(!calc.evaluate("Function('return 42')()").has_value());
        QVERIFY(!calc.evaluate("globalThis.Math.abs(-5)").has_value());
        QVERIFY(!calc.evaluate("__proto__").has_value());

        // Valid math functions must evaluate cleanly
        QCOMPARE(calc.evaluate("sqrt(256)").value_or(""), QStringLiteral("16"));
        QCOMPARE(calc.evaluate("sin(0)").value_or(""), QStringLiteral("0"));
        QCOMPARE(calc.evaluate("abs(-42)").value_or(""), QStringLiteral("42"));
        QCOMPARE(calc.evaluate("2^10").value_or(""), QStringLiteral("1024"));
    }
};

QTEST_MAIN(TestFuzzyMatcher)
#include "test_fuzzymatcher.moc"
