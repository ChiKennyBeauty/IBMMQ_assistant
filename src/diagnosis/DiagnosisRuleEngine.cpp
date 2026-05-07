#include "diagnosis/DiagnosisRuleEngine.h"

namespace {

bool containsAnyToken(const QString &text, const QStringList &tokens) {
  for (const QString &token : tokens) {
    if (text.contains(token, Qt::CaseInsensitive)) {
      return true;
    }
  }
  return false;
}

}  // namespace

DiagnosisResult DiagnosisRuleEngine::diagnose(const DiagnosisContext &context) const {
  DiagnosisResult result;

  const bool isConnectScene = context.scene.compare("connect", Qt::CaseInsensitive) == 0;
  const bool isReason2009 = context.errorCode == 2009;
  const bool hasTimeoutText =
      containsAnyToken(context.errorText, {"timeout", "timed out", "time out", "超时"});

  if (isConnectScene && isReason2009 && hasTimeoutText) {
    result.matched = true;
    result.faultType = "ConnectionTimeout";
    result.confidence = 88;
    result.evidence << "scene=connect" << "reason code=2009" << "error text contains timeout keyword";
    result.actions << "Check network connectivity to queue manager host and port."
                   << "Validate firewall/NAT idle timeout and keepalive settings."
                   << "Increase connect/channel timeout or retry with backoff.";
    return result;
  }

  const bool weakSignal = containsAnyToken(context.errorText, {"slow", "intermittent", "occasionally"}) ||
                          context.attrs.value("signalStrength").compare("weak", Qt::CaseInsensitive) == 0;
  if (weakSignal) {
    result.matched = false;
    result.faultType = "UncertainNetworkIssue";
    result.confidence = 35;
    result.evidence << "weak signal indicators observed";
    result.actions << "Collect additional connection logs and latency metrics.";
    return result;
  }

  return result;
}
