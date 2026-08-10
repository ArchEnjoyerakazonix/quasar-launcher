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

        // Acronym query: "ff" -> "Firefox Web Browser"
        matcher.setQuery("ff");
        QCOMPARE(matcher.rowCount() > 0, true);
        QModelIndex idx2 = matcher.index(0, 0);
        QString matchedName2 = matcher.data(idx2, Qt::UserRole + 1).toString();
        QCOMPARE(matchedName2, QString("Firefox Web Browser"));
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
};

QTEST_MAIN(TestFuzzyMatcher)
#include "test_fuzzymatcher.moc"
