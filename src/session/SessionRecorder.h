#pragma once

#include <QString>

#include "session/SessionTypes.h"

class SessionRecorder {
public:
  void startSession(const QString &envTag);
  void recordEvent(const SessionEvent &event);
  QString saveToFile(const QString &path) const;
  SessionData current() const;

private:
  SessionData session_;
};
