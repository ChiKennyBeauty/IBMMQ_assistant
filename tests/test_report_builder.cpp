#include <QtTest/QtTest>

#include "diagnosis/DiagnosisTypes.h"
#include "report/ReportBuilder.h"
#include "session/SessionTypes.h"

class ReportBuilderTest : public QObject {
  Q_OBJECT

private slots:
  void markdownContainsTitleAndKeyFields() {
    SessionData session;
    session.sessionId = "session-123";
    session.envTag = "sit";

    SessionEvent firstEvent;
    firstEvent.eventType = "connect";
    session.events.append(firstEvent);

    SessionEvent secondEvent;
    secondEvent.eventType = "browse";
    session.events.append(secondEvent);

    DiagnosisResult diagnosis;
    diagnosis.faultType = "ConnectionTimeout";
    diagnosis.confidence = 85;

    const QVector<DiagnosisResult> diagnoses{diagnosis};
    const ReportBuilder builder;
    const QString markdown = builder.buildMarkdown(session, diagnoses);

    QVERIFY(markdown.contains("# IBMMQ \u8C03\u8BD5\u56DE\u653E\u62A5\u544A"));
    QVERIFY(markdown.contains("\u4F1A\u8BDD ID: session-123"));
    QVERIFY(markdown.contains("\u73AF\u5883: sit"));
    QVERIFY(markdown.contains("\u4E8B\u4EF6\u603B\u6570: 2"));
    QVERIFY(markdown.contains("ConnectionTimeout"));
    QVERIFY(markdown.contains("\u7F6E\u4FE1\u5EA6: 85"));
  }

  void markdownShowsEventsEvenWithoutDiagnoses() {
    SessionData session;
    session.sessionId = "session-empty-diagnosis";
    session.envTag = "dev";

    SessionEvent connectEvent;
    connectEvent.eventType = "onConnected";
    connectEvent.ok = true;
    connectEvent.inputSnapshot.insert("queueManager", "QM1");
    session.events.append(connectEvent);

    SessionEvent browseEvent;
    browseEvent.eventType = "onMessagesBrowsed";
    browseEvent.ok = true;
    browseEvent.inputSnapshot.insert("queueName", "Q1");
    browseEvent.inputSnapshot.insert("itemCount", 2);
    session.events.append(browseEvent);

    const QVector<DiagnosisResult> diagnoses;
    const ReportBuilder builder;
    const QString markdown = builder.buildMarkdown(session, diagnoses);

    QVERIFY(markdown.contains("## \u8BCA\u65AD\u7ED3\u679C"));
    QVERIFY(markdown.contains("\u5F53\u524D\u4F1A\u8BDD\u672A\u751F\u6210\u8BCA\u65AD\u7ED3\u679C"));
    QVERIFY(markdown.contains("## \u4F1A\u8BDD\u4E8B\u4EF6"));
    QVERIFY(markdown.contains("onConnected"));
    QVERIFY(markdown.contains("onMessagesBrowsed"));
    QVERIFY(markdown.contains("queueName=Q1"));
    QVERIFY(markdown.contains("itemCount=2"));
  }
};

QTEST_MAIN(ReportBuilderTest)
#include "test_report_builder.moc"
