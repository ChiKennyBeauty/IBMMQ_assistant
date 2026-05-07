#pragma once

#include <QJsonObject>
#include <QMetaType>
#include <QString>

struct ConnectionConfig {
  QString name;
  QString queueManager;
  QString connName;
  QString channel;
  QString username;
  QString password;

  QJsonObject toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["queueManager"] = queueManager;
    json["connName"] = connName;
    json["channel"] = channel;
    json["username"] = username;
    json["password"] = password;
    return json;
  }

  static ConnectionConfig fromJson(const QJsonObject &json) {
    ConnectionConfig cfg;
    cfg.name = json.value("name").toString();
    cfg.queueManager = json.value("queueManager").toString();
    cfg.connName = json.value("connName").toString();
    cfg.channel = json.value("channel").toString();
    cfg.username = json.value("username").toString();
    cfg.password = json.value("password").toString();
    return cfg;
  }
};

Q_DECLARE_METATYPE(ConnectionConfig)
