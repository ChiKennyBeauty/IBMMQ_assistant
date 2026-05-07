#pragma once

#include "diagnosis/DiagnosisTypes.h"

class DiagnosisRuleEngine {
public:
  DiagnosisResult diagnose(const DiagnosisContext &context) const;
};
