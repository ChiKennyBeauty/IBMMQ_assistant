#include "session/SessionRecorder.h"

#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QUuid>

void SessionRecorder::startSession(const QString &envTag) {
  session_ = SessionData();
  session_.sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
  session_.envTag = envTag;
  session_.startAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

void SessionRecorder::recordEvent(const SessionEvent &event) {
  if (session_.startAt.isEmpty()) {
    startSession(QString());
  }
  session_.events.append(event);
  session_.endAt = event.timestamp.isEmpty()
                       ? QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)
                       : event.timestamp;
}

QString SessionRecorder::saveToFile(const QString &path) const {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return QString();
  }

  const QJsonDocument document(session_.toJson());
  if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
    return QString();
  }

  file.close();
  return path;
}

SessionData SessionRecorder::current() const { return session_; }
