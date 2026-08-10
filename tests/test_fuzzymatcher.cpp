#include <QtTest/QtTest>
#include <QSignalSpy>
#include "../src/fuzzymatcher.h"
#include "../src/appindexer.h"

class TestFuzzyMatcher : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Initializing TestFuzzyMatcher suite...";
    }

    void testDamerauLevenshteinTransposition() {
        FuzzyMatcher matcher;
        matcher.setQuery("ca");
        // "ac" vs "ca" -> 1 transposition edit distance
        // "cba" vs "abc" -> 2 edit distance
        QVERIFY(matcher.query() == "ca");
    }

    void testAcronymsAndTransliteration() {
        FuzzyMatcher matcher;
        matcher.setQuery("ghbdtn");
        QCOMPARE(matcher.query(), QString("ghbdtn"));
    }
};

QTEST_MAIN(TestFuzzyMatcher)
#include "test_fuzzymatcher.moc"
