#include <QtTest/QtTest>

#include "app/ConnectionConfig.h"

class ConnectionConfigTest : public QObject {
  Q_OBJECT

private slots:
  void roundTripKeepsValues() {
    ConnectionConfig in;
    in.name = "dev";
    in.queueManager = "QM1";
    in.connName = "127.0.0.1(1414)";
    in.channel = "DEV.APP.SVRCONN";
    in.username = "app";
    in.password = "pwd";

    const QJsonObject json = in.toJson();
    const ConnectionConfig out = ConnectionConfig::fromJson(json);

    QCOMPARE(out.name, QString("dev"));
    QCOMPARE(out.queueManager, QString("QM1"));
    QCOMPARE(out.connName, QString("127.0.0.1(1414)"));
    QCOMPARE(out.channel, QString("DEV.APP.SVRCONN"));
    QCOMPARE(out.username, QString("app"));
    QCOMPARE(out.password, QString("pwd"));
  }
};

QTEST_MAIN(ConnectionConfigTest)
#include "test_connection_config.moc"
