#include <QtTest/QtTest>

#include "diagnosis/DiagnosisRuleEngine.h"

class DiagnosisRuleEngineTest : public QObject {
  Q_OBJECT

private slots:
  void connect2009TimeoutMatchesWithHighConfidence() {
    DiagnosisRuleEngine engine;
    DiagnosisContext context;
    context.scene = "connect";
    context.errorCode = 2009;
    context.errorText = "MQCONN failed: operation timed out while handshaking";

    const DiagnosisResult result = engine.diagnose(context);

    QVERIFY(result.matched);
    QCOMPARE(result.faultType, QString("ConnectionTimeout"));
    QVERIFY(result.confidence >= 80);
    QVERIFY(!result.evidence.isEmpty());
    QVERIFY(!result.actions.isEmpty());
  }

  void weakSignalStaysLowConfidence() {
    DiagnosisRuleEngine engine;
    DiagnosisContext context;
    context.scene = "connect";
    context.errorCode = 0;
    context.errorText = "Network seems slow and intermittent";
    context.attrs.insert("signalStrength", "weak");

    const DiagnosisResult result = engine.diagnose(context);

    QVERIFY(!result.matched || result.confidence < 50);
    QVERIFY(result.confidence < 50);
  }
};

QTEST_MAIN(DiagnosisRuleEngineTest)
#include "test_diagnosis_engine.moc"
