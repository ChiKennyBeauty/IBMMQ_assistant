#pragma once

#include <QString>
#include <QVector>

#include "session/SessionTypes.h"

struct ReplayDelta {
  QString step;
  QString history;
  QString current;
};

class ReplayService {
public:
  QVector<ReplayDelta> replayReadOnly(const SessionData &session) const;
};
