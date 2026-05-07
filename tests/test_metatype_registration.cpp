#include <QtTest/QtTest>

#include "app/MetaTypeRegistry.h"

class MetaTypeRegistrationTest : public QObject {
  Q_OBJECT

private slots:
  void registersQueuedSignalTypes() {
    QVERIFY(QMetaType::type("ConnectionConfig") == QMetaType::UnknownType);

    registerAppMetaTypes();

    QVERIFY(QMetaType::type("ConnectionConfig") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("ConnectResult") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("ObjectListResult") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("BrowseResult") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("SendResult") != QMetaType::UnknownType);
    QVERIFY(QMetaType::type("ReceiveResult") != QMetaType::UnknownType);
  }
};

QTEST_MAIN(MetaTypeRegistrationTest)
#include "test_metatype_registration.moc"
