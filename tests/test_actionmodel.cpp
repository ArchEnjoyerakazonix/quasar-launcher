#include <QtTest>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>

#include "../src/actionmodel.h"

class TestActionModel : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        QStandardPaths::setTestModeEnabled(true);
        writeActionsJson();
    }

    void cleanupTestCase() {
        QFile::remove(configPath());
    }

    void loadsActionsFromConfig() {
        ActionModel model;
        // /hello is a plain command; /menu, /submenu and /echo are pipes.
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.data(model.index(0, 0), ActionModel::NameRole).toString(),
                 QStringLiteral("/hello"));
        QCOMPARE(model.data(model.index(0, 0), ActionModel::CommentRole).toString(),
                 QStringLiteral("Say hello"));
    }

    void pipeActionDetection() {
        ActionModel model;
        QVERIFY(model.isPipeAction("/menu"));
        QVERIFY(model.isPipeAction("/submenu"));
        QVERIFY(!model.isPipeAction("/hello"));
        QVERIFY(!model.isPipeAction("/nonexistent"));
        QCOMPARE(model.pipeActionNames(),
                 QStringList({QStringLiteral("/menu"), QStringLiteral("/submenu"),
                              QStringLiteral("/echo")}));
    }

    void pipeFirstRoundListsStdoutLines() {
        ActionModel model;

        model.triggerPipeAction("/menu");
        QTRY_COMPARE_WITH_TIMEOUT(model.pipeResultModel()->rowCount(), 3, 5000);

        QAbstractListModel *pipe = model.pipeResultModel();
        QCOMPARE(pipe->data(pipe->index(1, 0), Qt::UserRole + 1).toString(),
                 QStringLiteral("Two"));
        // ExecRole carries the selection marker for QML routing.
        QCOMPARE(pipe->data(pipe->index(0, 0), Qt::UserRole + 2).toString(),
                 QStringLiteral("__pipe__:One"));
        QVERIFY(model.isPipeActive());
    }

    void pipeRoundTripReplacesMenu() {
        ActionModel model;
        QSignalSpy menuSpy(&model, &ActionModel::pipeMenuUpdated);

        model.triggerPipeAction("/submenu");
        QTRY_COMPARE(model.pipeResultModel()->rowCount(), 3);

        // Selecting "A" must re-invoke the script with $1=A → sub-menu X/Y.
        model.selectPipeItem("A");
        QVERIFY(menuSpy.wait(5000));
        QAbstractListModel *pipe = model.pipeResultModel();
        QCOMPARE(pipe->rowCount(), 2);
        QCOMPARE(pipe->data(pipe->index(0, 0), Qt::UserRole + 1).toString(),
                 QStringLiteral("X"));
        QVERIFY(model.isPipeActive());
    }

    void pipeRoundTripEmptyStdoutCloses() {
        ActionModel model;
        QSignalSpy doneSpy(&model, &ActionModel::pipeActionDone);

        model.triggerPipeAction("/submenu");
        QTRY_COMPARE(model.pipeResultModel()->rowCount(), 3);

        // The script prints nothing for $1=X: action performed → close.
        model.selectPipeItem("X");
        QVERIFY(doneSpy.wait(5000));
        QCOMPARE(model.pipeResultModel()->rowCount(), 0);
        QVERIFY(!model.isPipeActive());
    }

    void inputQueryPipePassesRemainderAsArg() {
        ActionModel model;
        model.triggerPipeAction("/echo", QStringLiteral("payload"));
        QTRY_COMPARE_WITH_TIMEOUT(model.pipeResultModel()->rowCount(), 1, 5000);
        QCOMPARE(model.pipeResultModel()->data(model.pipeResultModel()->index(0, 0),
                                               Qt::UserRole + 1).toString(),
                 QStringLiteral("payload"));
        // "input": "query" disables remainder filtering in the matcher.
        QVERIFY(!model.pipeFiltersByRemainder());
    }

private:
    static QString configPath() {
        return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
               + "/quasar/actions.json";
    }

    static void writeActionsJson() {
        QDir().mkpath(QFileInfo(configPath()).absolutePath());

        QJsonArray actions;
        actions.append(QJsonObject{
            {"name", "/hello"},
            {"command", "printf hello"},
            {"icon", "utilities-terminal"},
            {"description", "Say hello"},
        });
        actions.append(QJsonObject{
            {"name", "/menu"},
            {"type", "pipe"},
            {"command", "printf 'One\\nTwo\\nThree'"},
            {"icon", "list-view"},
            {"description", "Simple menu"},
        });
        // Multi-step script: no args → menu; $1=A → sub-menu; $1=X → done.
        actions.append(QJsonObject{
            {"name", "/submenu"},
            {"type", "pipe"},
            {"command",
             "if [ \"$1\" = \"A\" ]; then printf 'X\\nY'; "
             "elif [ -z \"$1\" ]; then printf 'A\\nB\\nC'; fi"},
            {"icon", "view-list-tree"},
            {"description", "Multi-step menu"},
        });
        actions.append(QJsonObject{
            {"name", "/echo"},
            {"type", "pipe"},
            {"input", "query"},
            {"command", "printf '%s\\n' \"$1\""},
            {"icon", "edit-copy"},
            {"description", "Echoes the query"},
        });

        QFile f(configPath());
        f.open(QIODevice::WriteOnly);
        f.write(QJsonDocument(actions).toJson());
        f.close();
    }
};

QTEST_MAIN(TestActionModel)
#include "test_actionmodel.moc"
