#include "mq/MqWorker.h"

#include <imqi.hpp>

#include "mq/MessageFormatter.h"

MqWorker::MqWorker(QObject *parent) : QObject(parent) {}

void MqWorker::connectToQueueManager(ConnectionConfig cfg) {
  ImqQueueManager manager;
  ImqChannel *channel = nullptr;

  const QByteArray qmName = cfg.queueManager.toLocal8Bit();
  if (!qmName.isEmpty()) {
    manager.setName(qmName.constData());
  }

  // If both channel and connName are present, use client mode with TCP.
  if (!cfg.channel.isEmpty() && !cfg.connName.isEmpty()) {
    channel = new ImqChannel();
    const QByteArray channelName = cfg.channel.toLocal8Bit();
    const QByteArray connName = cfg.connName.toLocal8Bit();
    channel->setChannelName(channelName.constData());
    channel->setConnectionName(connName.constData());
    channel->setTransportType(MQXPT_TCP);
    manager.setChannelReference(channel);
  }

  ConnectResult result;
  result.ok = manager.connect();
  if (!result.ok) {
    result.error.where = "connect";
    result.error.code = static_cast<int>(manager.reasonCode());
    result.error.reasonText = QString("MQ connect failed");
  } else {
    manager.disconnect();
  }

  if (channel) {
    manager.setChannelReference();
    delete channel;
  }

  emit connected(result);
}

void MqWorker::listObjects(ConnectionConfig cfg) {
  ObjectListResult result;

  ImqQueueManager manager;
  ImqChannel *channel = nullptr;

  const QByteArray qmName = cfg.queueManager.toLocal8Bit();
  if (!qmName.isEmpty()) {
    manager.setName(qmName.constData());
  }

  if (!cfg.channel.isEmpty() && !cfg.connName.isEmpty()) {
    channel = new ImqChannel();
    const QByteArray channelName = cfg.channel.toLocal8Bit();
    const QByteArray connName = cfg.connName.toLocal8Bit();
    channel->setChannelName(channelName.constData());
    channel->setConnectionName(connName.constData());
    channel->setTransportType(MQXPT_TCP);
    manager.setChannelReference(channel);
  }

  if (!manager.connect()) {
    result.ok = false;
    result.error.where = "listObjects";
    result.error.code = static_cast<int>(manager.reasonCode());
    result.error.reasonText = "MQ list objects failed: connect failed";
  } else {
    result.ok = true;

    // MVP: expose stable system objects and active connection channel.
    ObjectItem q1;
    q1.type = "Queue";
    q1.name = "SYSTEM.DEFAULT.LOCAL.QUEUE";
    q1.attrs.insert("source", "mvp-default");
    result.items.push_back(q1);

    ObjectItem q2;
    q2.type = "Queue";
    q2.name = "SYSTEM.ADMIN.COMMAND.QUEUE";
    q2.attrs.insert("source", "mvp-default");
    result.items.push_back(q2);

    ObjectItem ch;
    ch.type = "Channel";
    ch.name = cfg.channel.isEmpty() ? "SYSTEM.DEF.SVRCONN" : cfg.channel;
    ch.attrs.insert("connName", cfg.connName);
    result.items.push_back(ch);

    manager.disconnect();
  }

  if (channel) {
    manager.setChannelReference();
    delete channel;
  }

  emit objectsListed(result);
}

void MqWorker::browseMessages(ConnectionConfig cfg, QString queueName, int offset, int pageSize) {
  BrowseResult result;
  result.nextOffset = offset;

  ImqQueueManager manager;
  ImqChannel *channel = nullptr;

  const QByteArray qmName = cfg.queueManager.toLocal8Bit();
  if (!qmName.isEmpty()) {
    manager.setName(qmName.constData());
  }

  if (!cfg.channel.isEmpty() && !cfg.connName.isEmpty()) {
    channel = new ImqChannel();
    const QByteArray channelName = cfg.channel.toLocal8Bit();
    const QByteArray connName = cfg.connName.toLocal8Bit();
    channel->setChannelName(channelName.constData());
    channel->setConnectionName(connName.constData());
    channel->setTransportType(MQXPT_TCP);
    manager.setChannelReference(channel);
  }

  if (!manager.connect()) {
    result.ok = false;
    result.error.where = "browseMessages";
    result.error.code = static_cast<int>(manager.reasonCode());
    result.error.reasonText = "MQ browse failed: connect failed";
    emit messagesBrowsed(result);
    if (channel) {
      manager.setChannelReference();
      delete channel;
    }
    return;
  }

  ImqQueue queue;
  queue.setConnectionReference(manager);
  const QByteArray qName = queueName.toLocal8Bit();
  queue.setName(qName.isEmpty() ? "SYSTEM.DEFAULT.LOCAL.QUEUE" : qName.constData());
  queue.setOpenOptions(MQOO_BROWSE | MQOO_FAIL_IF_QUIESCING);
  queue.open();
  if (queue.completionCode() == MQCC_FAILED) {
    result.ok = false;
    result.error.where = "browseMessages";
    result.error.code = static_cast<int>(queue.reasonCode());
    result.error.reasonText = "MQ browse failed: queue open failed";
  } else {
    ImqMessage msg;
    char buffer[8192];
    msg.useEmptyBuffer(buffer, sizeof(buffer));

    ImqGetMessageOptions gmo;
    gmo.setOptions(MQGMO_BROWSE_FIRST | MQGMO_FAIL_IF_QUIESCING);

    int index = 0;
    const int limit = offset + pageSize + 1;
    while (index < limit) {
      msg.setMessageId();
      msg.setCorrelationId();
      const bool ok = queue.get(msg, gmo);
      if (!ok) {
        if (queue.reasonCode() == MQRC_NO_MSG_AVAILABLE) {
          break;
        }
        result.ok = false;
        result.error.where = "browseMessages";
        result.error.code = static_cast<int>(queue.reasonCode());
        result.error.reasonText = "MQ browse failed: get failed";
        break;
      }

      const QByteArray payload(msg.bufferPointer(), static_cast<int>(msg.dataLength()));
      if (index >= offset && index < offset + pageSize) {
        MessageItem item;
        item.textPreview = toTextPreview(payload);
        item.hexPreview = toHexPreview(payload);
        result.page.push_back(item);
      }
      index++;
      gmo.setOptions(MQGMO_BROWSE_NEXT | MQGMO_FAIL_IF_QUIESCING);
    }

    if (result.error.reasonText.isEmpty()) {
      result.ok = true;
      result.hasMore = (index > offset + pageSize);
      result.nextOffset = offset + result.page.size();
    }
    queue.close();
  }

  manager.disconnect();
  if (channel) {
    manager.setChannelReference();
    delete channel;
  }
  emit messagesBrowsed(result);
}

void MqWorker::sendMessage(ConnectionConfig cfg, QString queueName, QString body) {
  SendResult result;
  result.queueName = queueName.isEmpty() ? "SYSTEM.DEFAULT.LOCAL.QUEUE" : queueName;

  ImqQueueManager manager;
  ImqChannel *channel = nullptr;
  const QByteArray qmName = cfg.queueManager.toLocal8Bit();
  if (!qmName.isEmpty()) {
    manager.setName(qmName.constData());
  }
  if (!cfg.channel.isEmpty() && !cfg.connName.isEmpty()) {
    channel = new ImqChannel();
    const QByteArray channelName = cfg.channel.toLocal8Bit();
    const QByteArray connName = cfg.connName.toLocal8Bit();
    channel->setChannelName(channelName.constData());
    channel->setConnectionName(connName.constData());
    channel->setTransportType(MQXPT_TCP);
    manager.setChannelReference(channel);
  }

  if (!manager.connect()) {
    result.ok = false;
    result.error.where = "sendMessage";
    result.error.code = static_cast<int>(manager.reasonCode());
    result.error.reasonText = "MQ send failed: connect failed";
    emit messageSent(result);
    if (channel) {
      manager.setChannelReference();
      delete channel;
    }
    return;
  }

  ImqQueue queue;
  queue.setConnectionReference(manager);
  const QByteArray qName = result.queueName.toLocal8Bit();
  queue.setName(qName.constData());
  queue.setOpenOptions(MQOO_OUTPUT | MQOO_FAIL_IF_QUIESCING);
  queue.open();
  if (queue.completionCode() == MQCC_FAILED) {
    result.ok = false;
    result.error.where = "sendMessage";
    result.error.code = static_cast<int>(queue.reasonCode());
    result.error.reasonText = "MQ send failed: queue open failed";
  } else {
    ImqMessage msg;
    const QByteArray payload = body.toUtf8();
    msg.write(static_cast<size_t>(payload.size()), payload.constData());
    const bool putOk = queue.put(msg);
    if (!putOk) {
      result.ok = false;
      result.error.where = "sendMessage";
      result.error.code = static_cast<int>(queue.reasonCode());
      result.error.reasonText = "MQ send failed: put failed";
    } else {
      result.ok = true;
      result.bytes = payload.size();
    }
    queue.close();
  }

  manager.disconnect();
  if (channel) {
    manager.setChannelReference();
    delete channel;
  }
  emit messageSent(result);
}

void MqWorker::receiveMessage(ConnectionConfig cfg, QString queueName) {
  ReceiveResult result;
  result.queueName = queueName.isEmpty() ? "SYSTEM.DEFAULT.LOCAL.QUEUE" : queueName;

  ImqQueueManager manager;
  ImqChannel *channel = nullptr;
  const QByteArray qmName = cfg.queueManager.toLocal8Bit();
  if (!qmName.isEmpty()) {
    manager.setName(qmName.constData());
  }
  if (!cfg.channel.isEmpty() && !cfg.connName.isEmpty()) {
    channel = new ImqChannel();
    const QByteArray channelName = cfg.channel.toLocal8Bit();
    const QByteArray connName = cfg.connName.toLocal8Bit();
    channel->setChannelName(channelName.constData());
    channel->setConnectionName(connName.constData());
    channel->setTransportType(MQXPT_TCP);
    manager.setChannelReference(channel);
  }

  if (!manager.connect()) {
    result.ok = false;
    result.error.where = "receiveMessage";
    result.error.code = static_cast<int>(manager.reasonCode());
    result.error.reasonText = "MQ receive failed: connect failed";
    emit messageReceived(result);
    if (channel) {
      manager.setChannelReference();
      delete channel;
    }
    return;
  }

  ImqQueue queue;
  queue.setConnectionReference(manager);
  const QByteArray qName = result.queueName.toLocal8Bit();
  queue.setName(qName.constData());
  queue.setOpenOptions(MQOO_INPUT_AS_Q_DEF | MQOO_FAIL_IF_QUIESCING);
  queue.open();
  if (queue.completionCode() == MQCC_FAILED) {
    result.ok = false;
    result.error.where = "receiveMessage";
    result.error.code = static_cast<int>(queue.reasonCode());
    result.error.reasonText = "MQ receive failed: queue open failed";
  } else {
    ImqMessage msg;
    char buffer[8192];
    msg.useEmptyBuffer(buffer, sizeof(buffer));

    ImqGetMessageOptions gmo;
    gmo.setOptions(MQGMO_WAIT | MQGMO_FAIL_IF_QUIESCING);
    gmo.setWaitInterval(1000);

    if (!queue.get(msg, gmo)) {
      if (queue.reasonCode() == MQRC_NO_MSG_AVAILABLE) {
        result.ok = true;
        result.hasMessage = false;
      } else {
        result.ok = false;
        result.error.where = "receiveMessage";
        result.error.code = static_cast<int>(queue.reasonCode());
        result.error.reasonText = "MQ receive failed: get failed";
      }
    } else {
      const QByteArray payload(msg.bufferPointer(), static_cast<int>(msg.dataLength()));
      result.ok = true;
      result.hasMessage = true;
      result.message.textPreview = toTextPreview(payload);
      result.message.hexPreview = toHexPreview(payload);
    }
    queue.close();
  }

  manager.disconnect();
  if (channel) {
    manager.setChannelReference();
    delete channel;
  }
  emit messageReceived(result);
}
