#include "session/ReplayService.h"

#include <QStringList>

namespace {
bool isReadOnlyEvent(const QString &eventType) {
  return eventType == "onConnected" || eventType == "onObjectsListed" || eventType == "onMessagesBrowsed";
}

QString eventSummary(const SessionEvent &event) {
  const QString status = event.ok ? "ok" : "fail";
  if (event.eventType == "onConnected") {
    const QString qmgr = event.inputSnapshot.value("queueManager").toString();
    return QString("connect %1 qmgr=%2").arg(status, qmgr);
  }
  if (event.eventType == "onObjectsListed") {
    const int count = event.inputSnapshot.value("itemCount").toInt();
    return QString("list %1 count=%2").arg(status).arg(count);
  }
  if (event.eventType == "onMessagesBrowsed") {
    const QString queueName = event.inputSnapshot.value("queueName").toString();
    const int count = event.inputSnapshot.value("itemCount").toInt();
    return QString("browse %1 queue=%2 count=%3").arg(status, queueName).arg(count);
  }
  return QString("%1 %2").arg(event.eventType, status);
}
} // namespace

QVector<ReplayDelta> ReplayService::replayReadOnly(const SessionData &session) const {
  QVector<ReplayDelta> deltas;
  QStringList history;
  int stepIndex = 1;
  for (const SessionEvent &event : session.events) {
    if (!isReadOnlyEvent(event.eventType)) {
      continue;
    }

    ReplayDelta delta;
    delta.step = QString("step-%1").arg(stepIndex++);
    delta.history = history.isEmpty() ? "none" : history.join(" -> ");
    delta.current = eventSummary(event);
    history.append(delta.current);
    deltas.append(delta);
  }
  return deltas;
}
