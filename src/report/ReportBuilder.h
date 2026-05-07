#pragma once

#include <QString>
#include <QVector>

#include "diagnosis/DiagnosisTypes.h"
#include "session/SessionTypes.h"

class ReportBuilder {
public:
  QString buildMarkdown(const SessionData &session, const QVector<DiagnosisResult> &diagnoses) const;
};
