#pragma once

#include <QMap>
#include <QMetaType>
#include <QString>
#include <QVector>

struct MqError {
  int code = 0;
  QString reasonText;
  QString where;
};

struct ConnectResult {
  bool ok = false;
  MqError error;
};

struct ObjectItem {
  QString type;
  QString name;
  QMap<QString, QString> attrs;
};

struct ObjectListResult {
  bool ok = false;
  QVector<ObjectItem> items;
  MqError error;
};

struct MessageItem {
  QString textPreview;
  QString hexPreview;
};

struct BrowseResult {
  bool ok = false;
  QVector<MessageItem> page;
  int nextOffset = 0;
  bool hasMore = false;
  MqError error;
};

struct SendResult {
  bool ok = false;
  QString queueName;
  int bytes = 0;
  MqError error;
};

struct ReceiveResult {
  bool ok = false;
  QString queueName;
  MessageItem message;
  bool hasMessage = false;
  MqError error;
};

Q_DECLARE_METATYPE(MqError)
Q_DECLARE_METATYPE(ConnectResult)
Q_DECLARE_METATYPE(ObjectItem)
Q_DECLARE_METATYPE(ObjectListResult)
Q_DECLARE_METATYPE(MessageItem)
Q_DECLARE_METATYPE(BrowseResult)
Q_DECLARE_METATYPE(SendResult)
Q_DECLARE_METATYPE(ReceiveResult)
