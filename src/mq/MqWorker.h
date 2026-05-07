#pragma once

#include <QObject>

#include "app/ConnectionConfig.h"
#include "mq/MqTypes.h"

class MqWorker : public QObject {
  Q_OBJECT

public:
  explicit MqWorker(QObject *parent = nullptr);

public slots:
  void connectToQueueManager(ConnectionConfig cfg);
  void listObjects(ConnectionConfig cfg);
  void browseMessages(ConnectionConfig cfg, QString queueName, int offset, int pageSize);
  void sendMessage(ConnectionConfig cfg, QString queueName, QString body);
  void receiveMessage(ConnectionConfig cfg, QString queueName);

signals:
  void connected(ConnectResult result);
  void objectsListed(ObjectListResult result);
  void messagesBrowsed(BrowseResult result);
  void messageSent(SendResult result);
  void messageReceived(ReceiveResult result);
};
