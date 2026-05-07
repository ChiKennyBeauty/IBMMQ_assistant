#pragma once

#include <QMap>
#include <QString>
#include <QStringList>

struct DiagnosisContext {
  QString scene;
  int errorCode = 0;
  QString errorText;
  QMap<QString, QString> attrs;
};

struct DiagnosisResult {
  bool matched = false;
  QString faultType;
  int confidence = 0;
  QStringList evidence;
  QStringList actions;
};
