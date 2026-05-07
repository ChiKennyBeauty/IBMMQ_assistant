#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

struct SessionEvent {
  QString eventType;
  QString timestamp;
  QJsonObject inputSnapshot;
  bool ok = false;
  qint64 durationMs = 0;
  QString errorText;
  int errorCode = 0;

  QJsonObject toJson() const {
    QJsonObject json;
    json["eventType"] = eventType;
    json["timestamp"] = timestamp;
    json["inputSnapshot"] = inputSnapshot;
    json["ok"] = ok;
    json["durationMs"] = static_cast<double>(durationMs);
    json["errorText"] = errorText;
    json["errorCode"] = errorCode;
    return json;
  }
};

struct SessionData {
  QString sessionId;
  QString envTag;
  QString startAt;
  QString endAt;
  QVector<SessionEvent> events;

  QJsonObject toJson() const {
    QJsonObject json;
    json["sessionId"] = sessionId;
    json["envTag"] = envTag;
    json["startAt"] = startAt;
    json["endAt"] = endAt;

    QJsonArray eventsArray;
    for (const SessionEvent &event : events) {
      eventsArray.append(event.toJson());
    }
    json["events"] = eventsArray;
    return json;
  }
};
