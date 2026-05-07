#include "report/ReportBuilder.h"

#include <QJsonDocument>
#include <QStringList>

namespace {
QString formatSnapshot(const QJsonObject &snapshot) {
  if (snapshot.isEmpty()) {
    return "none";
  }

  QStringList pairs;
  for (auto it = snapshot.constBegin(); it != snapshot.constEnd(); ++it) {
    if (it.value().isString()) {
      pairs.append(QString("%1=%2").arg(it.key(), it.value().toString()));
      continue;
    }
    if (it.value().isDouble()) {
      pairs.append(QString("%1=%2").arg(it.key()).arg(it.value().toDouble(), 0, 'g', 15));
      continue;
    }
    if (it.value().isBool()) {
      pairs.append(QString("%1=%2").arg(it.key(), it.value().toBool() ? "true" : "false"));
      continue;
    }
    if (it.value().isObject()) {
      pairs.append(QString("%1=%2").arg(
          it.key(), QString::fromUtf8(QJsonDocument(it.value().toObject()).toJson(QJsonDocument::Compact))));
      continue;
    }
    if (it.value().isArray()) {
      pairs.append(QString("%1=%2").arg(
          it.key(), QString::fromUtf8(QJsonDocument(it.value().toArray()).toJson(QJsonDocument::Compact))));
      continue;
    }
    pairs.append(QString("%1=null").arg(it.key()));
  }
  return pairs.join(", ");
}
} // namespace

QString ReportBuilder::buildMarkdown(const SessionData &session, const QVector<DiagnosisResult> &diagnoses) const {
  QString markdown;
  markdown += "# IBMMQ \u8C03\u8BD5\u56DE\u653E\u62A5\u544A\n\n";
  markdown += QString("\u4F1A\u8BDD ID: %1\n").arg(session.sessionId);
  markdown += QString("\u73AF\u5883: %1\n").arg(session.envTag);
  markdown += QString("\u4E8B\u4EF6\u603B\u6570: %1\n\n").arg(session.events.size());
  markdown += "## \u8BCA\u65AD\u7ED3\u679C\n";

  if (diagnoses.isEmpty()) {
    markdown += "- \u5F53\u524D\u4F1A\u8BDD\u672A\u751F\u6210\u8BCA\u65AD\u7ED3\u679C\u3002\n";
  }
  for (const DiagnosisResult &diagnosis : diagnoses) {
    markdown += QString("- \u6545\u969C\u7C7B\u578B: %1\n").arg(diagnosis.faultType);
    markdown += QString("  - \u7F6E\u4FE1\u5EA6: %1\n").arg(diagnosis.confidence);
  }

  markdown += "\n## \u4F1A\u8BDD\u4E8B\u4EF6\n";
  if (session.events.isEmpty()) {
    markdown += "- \u5F53\u524D\u4F1A\u8BDD\u672A\u8BB0\u5F55\u4EFB\u4F55\u4E8B\u4EF6\u3002\n";
  }
  for (const SessionEvent &event : session.events) {
    markdown += QString("- \u4E8B\u4EF6: %1").arg(event.eventType.isEmpty() ? "unknown" : event.eventType);
    if (!event.timestamp.isEmpty()) {
      markdown += QString("\uFF08\u65F6\u95F4: %1\uFF09").arg(event.timestamp);
    }
    markdown += "\n";
    markdown += QString("  - \u72B6\u6001: %1\n").arg(event.ok ? "\u6210\u529F" : "\u5931\u8D25");
    markdown += QString("  - \u5FEB\u7167: %1\n").arg(formatSnapshot(event.inputSnapshot));
    if (!event.errorText.isEmpty()) {
      markdown += QString("  - \u9519\u8BEF: %1 (code=%2)\n").arg(event.errorText).arg(event.errorCode);
    }
  }

  return markdown;
}
