#include <QtTest/QtTest>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "session/SessionRecorder.h"

class SessionRecorderTest : public QObject {
  Q_OBJECT

private slots:
  void startRecordAndSaveCreatesFile() {
    SessionRecorder recorder;
    recorder.startSession("dev");

    SessionEvent event;
    event.eventType = "connect";
    event.timestamp = "2026-05-07T12:34:56.789Z";
    event.inputSnapshot = QJsonObject{{"queueManager", "QM1"}};
    event.ok = true;
    event.durationMs = 120;
    event.errorText = "";
    event.errorCode = 0;
    recorder.recordEvent(event);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath("session.json");
    const QString savedPath = recorder.saveToFile(path);

    QCOMPARE(savedPath, path);
    QVERIFY(QFile::exists(path));
  }

  void serializedContentContainsKeyFields() {
    SessionRecorder recorder;
    recorder.startSession("sit");

    SessionEvent event;
    event.eventType = "browse";
    event.timestamp = "2026-05-07T12:35:10.001Z";
    event.inputSnapshot = QJsonObject{{"queue", "DEV.QUEUE.1"}};
    event.ok = false;
    event.durationMs = 35;
    event.errorText = "timeout";
    event.errorCode = 2009;
    recorder.recordEvent(event);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath("session_content.json");
    QVERIFY(!recorder.saveToFile(path).isEmpty());

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    QVERIFY(document.isObject());

    const QJsonObject root = document.object();
    QVERIFY(root.contains("sessionId"));
    QCOMPARE(root.value("envTag").toString(), QString("sit"));
    QVERIFY(root.contains("events"));

    const QJsonArray events = root.value("events").toArray();
    QCOMPARE(events.size(), 1);
    const QJsonObject firstEvent = events.first().toObject();
    QCOMPARE(firstEvent.value("eventType").toString(), QString("browse"));
  }
};

QTEST_MAIN(SessionRecorderTest)
#include "test_session_recorder.moc"
