# IBMMQ Intelligent Diagnosis + Replay Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a rule-based diagnosis engine and session replay/report capabilities to the current IBMMQ desktop app without regressing existing core features.

**Architecture:** Keep existing `MainWindow <-> MqWorker` flow. Introduce small focused modules for diagnosis, session recording, replay orchestration, and report export. Wire these modules through UI callbacks so each MQ action creates an event, failed actions trigger diagnosis, and users can replay/export from persisted sessions.

**Tech Stack:** C++17, Qt 5.14.2 (Core/Widgets/Test), existing MQ worker (`imqi.hpp` usage), CMake, Qt Test, ctest

---

## File Structure

- Create: `src/diagnosis/DiagnosisTypes.h` - diagnosis DTOs and rule schema
- Create: `src/diagnosis/DiagnosisRuleEngine.h` - engine interface
- Create: `src/diagnosis/DiagnosisRuleEngine.cpp` - rule match and scoring logic
- Create: `src/session/SessionTypes.h` - session and event models
- Create: `src/session/SessionRecorder.h` - session recorder interface
- Create: `src/session/SessionRecorder.cpp` - in-memory + JSON persistence
- Create: `src/session/ReplayService.h` - replay API
- Create: `src/session/ReplayService.cpp` - read-only replay workflow
- Create: `src/report/ReportBuilder.h` - report generation API
- Create: `src/report/ReportBuilder.cpp` - markdown report renderer
- Modify: `src/app/MainWindow.h` - add diagnosis/session/replay/report members and slots
- Modify: `src/app/MainWindow.cpp` - wire callbacks, new panels, export/replay actions
- Modify: `CMakeLists.txt` - compile new modules + tests
- Create: `tests/test_diagnosis_engine.cpp` - rules and scoring tests
- Create: `tests/test_session_recorder.cpp` - event persistence tests
- Create: `tests/test_report_builder.cpp` - report content tests

### Task 1: Add Diagnosis Core (Rules + Scoring)

**Files:**
- Create: `src/diagnosis/DiagnosisTypes.h`
- Create: `src/diagnosis/DiagnosisRuleEngine.h`
- Create: `src/diagnosis/DiagnosisRuleEngine.cpp`
- Create: `tests/test_diagnosis_engine.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write failing diagnosis tests**

```cpp
// tests/test_diagnosis_engine.cpp
#include <QtTest>
#include "diagnosis/DiagnosisRuleEngine.h"

class DiagnosisEngineTest final : public QObject {
  Q_OBJECT
private slots:
  void should_match_connect_timeout_rule();
  void should_output_low_confidence_when_only_weak_signal();
};

void DiagnosisEngineTest::should_match_connect_timeout_rule() {
  DiagnosisContext ctx;
  ctx.scene = "connect";
  ctx.errorCode = 2009;
  ctx.errorText = "MQ connect failed timeout";
  DiagnosisRuleEngine engine;
  const DiagnosisResult result = engine.diagnose(ctx);
  QVERIFY(result.matched);
  QCOMPARE(result.faultType, QString("ConnectionTimeout"));
  QVERIFY(result.confidence >= 80);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --config Debug && ctest -C Debug -R diagnosis_engine --output-on-failure`  
Expected: FAIL with missing diagnosis types/engine symbols

- [ ] **Step 3: Implement minimal diagnosis engine**

```cpp
// src/diagnosis/DiagnosisTypes.h
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
```

```cpp
// src/diagnosis/DiagnosisRuleEngine.cpp
DiagnosisResult DiagnosisRuleEngine::diagnose(const DiagnosisContext& ctx) const {
  DiagnosisResult out;
  if (ctx.scene == "connect" && ctx.errorCode == 2009) {
    out.matched = true;
    out.faultType = "ConnectionTimeout";
    out.confidence = 90;
    out.evidence = {"scene=connect", "code=2009"};
    out.actions = {"Check MQ listener port", "Check firewall", "Check channel status"};
  }
  return out;
}
```

- [ ] **Step 4: Re-run diagnosis tests**

Run: `cmake --build build --config Debug && ctest -C Debug -R diagnosis_engine --output-on-failure`  
Expected: PASS

- [ ] **Step 5: Commit**

Run: `git add src/diagnosis tests/test_diagnosis_engine.cpp CMakeLists.txt && git commit -m "feat: add rule-based diagnosis engine core"`

### Task 2: Add Session Recorder (Event Timeline + JSON)

**Files:**
- Create: `src/session/SessionTypes.h`
- Create: `src/session/SessionRecorder.h`
- Create: `src/session/SessionRecorder.cpp`
- Create: `tests/test_session_recorder.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write failing recorder tests**

```cpp
// tests/test_session_recorder.cpp
void SessionRecorderTest::should_persist_session_with_events() {
  SessionRecorder recorder;
  recorder.startSession("dev");
  recorder.recordEvent({"connect", QDateTime::currentDateTime(), {{"qm","QM1"}}, true, 45, "", 0});
  const QString path = recorder.saveToFile("build/test-session.json");
  QVERIFY(QFileInfo::exists(path));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --config Debug && ctest -C Debug -R session_recorder --output-on-failure`  
Expected: FAIL with missing session recorder symbols

- [ ] **Step 3: Implement minimal recorder + JSON serialization**

```cpp
// src/session/SessionTypes.h
struct SessionEvent {
  QString eventType;
  QDateTime timestamp;
  QMap<QString, QString> inputSnapshot;
  bool ok = false;
  int durationMs = 0;
  QString errorText;
  int errorCode = 0;
};
struct SessionData {
  QString sessionId;
  QString envTag;
  QDateTime startAt;
  QDateTime endAt;
  QVector<SessionEvent> events;
};
```

```cpp
// src/session/SessionRecorder.h
class SessionRecorder {
public:
  void startSession(const QString& envTag);
  void recordEvent(const SessionEvent& event);
  QString saveToFile(const QString& path) const;
  const SessionData& current() const;
};
```

- [ ] **Step 4: Re-run recorder tests**

Run: `cmake --build build --config Debug && ctest -C Debug -R session_recorder --output-on-failure`  
Expected: PASS

- [ ] **Step 5: Commit**

Run: `git add src/session tests/test_session_recorder.cpp CMakeLists.txt && git commit -m "feat: add session recorder with json persistence"`

### Task 3: Add Report Builder (Markdown Export)

**Files:**
- Create: `src/report/ReportBuilder.h`
- Create: `src/report/ReportBuilder.cpp`
- Create: `tests/test_report_builder.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write failing report tests**

```cpp
// tests/test_report_builder.cpp
void ReportBuilderTest::should_render_markdown_summary() {
  SessionData session;
  session.sessionId = "S-001";
  session.envTag = "test";
  ReportBuilder builder;
  const QString md = builder.buildMarkdown(session, {});
  QVERIFY(md.contains("# MQ Debug Replay Report"));
  QVERIFY(md.contains("S-001"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --config Debug && ctest -C Debug -R report_builder --output-on-failure`  
Expected: FAIL with missing report builder symbols

- [ ] **Step 3: Implement markdown builder**

```cpp
// src/report/ReportBuilder.h
class ReportBuilder {
public:
  QString buildMarkdown(const SessionData& session, const QVector<DiagnosisResult>& diagnoses) const;
};
```

```cpp
// src/report/ReportBuilder.cpp
QString ReportBuilder::buildMarkdown(const SessionData& s, const QVector<DiagnosisResult>& ds) const {
  QString out;
  out += "# MQ Debug Replay Report\n\n";
  out += QString("- Session: `%1`\n").arg(s.sessionId);
  out += QString("- Env: `%1`\n").arg(s.envTag);
  out += QString("- Events: `%1`\n\n").arg(s.events.size());
  out += "## Diagnosis\n";
  for (const auto& d : ds) {
    out += QString("- %1 (confidence=%2)\n").arg(d.faultType).arg(d.confidence);
  }
  return out;
}
```

- [ ] **Step 4: Re-run report tests**

Run: `cmake --build build --config Debug && ctest -C Debug -R report_builder --output-on-failure`  
Expected: PASS

- [ ] **Step 5: Commit**

Run: `git add src/report tests/test_report_builder.cpp CMakeLists.txt && git commit -m "feat: add markdown report builder for replay sessions"`

### Task 4: Integrate Diagnosis + Session Recording into Main UI Flow

**Files:**
- Modify: `src/app/MainWindow.h`
- Modify: `src/app/MainWindow.cpp`

- [ ] **Step 1: Write failing UI behavior test checklist**

```text
1) failed connect should produce diagnosis text
2) every connect/list/browse/send/receive callback should append session event
3) diagnosis panel should show confidence and actions
```

- [ ] **Step 2: Reproduce current missing behavior manually**

Run: `build/Debug/ibmmq_debug_assistant.exe`  
Expected: no diagnosis panel / no persisted session events yet

- [ ] **Step 3: Add minimal integration code**

```cpp
// MainWindow.h
DiagnosisRuleEngine diagnosisEngine_;
SessionRecorder sessionRecorder_;
QPlainTextEdit* diagnosisBox_ = nullptr;
```

```cpp
// MainWindow.cpp (example in onConnected)
SessionEvent ev;
ev.eventType = "connect";
ev.timestamp = QDateTime::currentDateTime();
ev.ok = result.ok;
ev.errorCode = result.error.code;
ev.errorText = result.error.reasonText;
sessionRecorder_.recordEvent(ev);
if (!result.ok) {
  DiagnosisContext ctx;
  ctx.scene = "connect";
  ctx.errorCode = result.error.code;
  ctx.errorText = result.error.reasonText;
  const DiagnosisResult d = diagnosisEngine_.diagnose(ctx);
  diagnosisBox_->setPlainText(QString("%1 (%2)").arg(d.faultType).arg(d.confidence));
}
```

- [ ] **Step 4: Build and run manual verification once**

Run: `cmake --build build --config Debug`  
Then run: `build/Debug/ibmmq_debug_assistant.exe`  
Expected: diagnosis panel updates on failed operations; actions recorded in session

- [ ] **Step 5: Commit**

Run: `git add src/app/MainWindow.h src/app/MainWindow.cpp && git commit -m "feat: integrate diagnosis and session recording into main flow"`

### Task 5: Add Replay Service + UI Replay/Export Actions

**Files:**
- Create: `src/session/ReplayService.h`
- Create: `src/session/ReplayService.cpp`
- Modify: `src/app/MainWindow.h`
- Modify: `src/app/MainWindow.cpp`

- [ ] **Step 1: Write failing replay checklist**

```text
1) can load saved session file
2) can replay connect/list/browse as read-only sequence
3) can display "current vs history" result delta summary
```

- [ ] **Step 2: Run app to confirm replay actions missing**

Run: `build/Debug/ibmmq_debug_assistant.exe`  
Expected: no replay button and no delta output

- [ ] **Step 3: Implement replay service + button handlers**

```cpp
// ReplayService.h
struct ReplayDelta { QString step; QString history; QString current; };
class ReplayService {
public:
  QVector<ReplayDelta> replayReadOnly(const SessionData& session, MqWorker* worker) const;
};
```

```cpp
// MainWindow.cpp (button handler)
const auto deltas = replayService_.replayReadOnly(loadedSession_, workerProxy_);
for (const auto& d : deltas) {
  appendLog(QString("回放差异: %1 | 历史=%2 | 当前=%3").arg(d.step, d.history, d.current));
}
```

- [ ] **Step 4: Build and run manual replay once**

Run: `cmake --build build --config Debug`  
Then run: `build/Debug/ibmmq_debug_assistant.exe`  
Expected: replay works for read-only steps and prints delta summary

- [ ] **Step 5: Commit**

Run: `git add src/session/ReplayService.* src/app/MainWindow.* && git commit -m "feat: add read-only replay service and delta output"`

### Task 6: Export Full Replay Report + Final Verification

**Files:**
- Modify: `src/app/MainWindow.cpp`
- Modify: `README.md`

- [ ] **Step 1: Add failing behavior checklist**

```text
1) export button writes markdown report
2) report includes session summary, diagnosis list, action suggestions
3) report path visible in UI log
```

- [ ] **Step 2: Verify missing behavior before implementation**

Run: `build/Debug/ibmmq_debug_assistant.exe`  
Expected: no full replay report export workflow yet

- [ ] **Step 3: Implement export action**

```cpp
// MainWindow.cpp
const QString report = reportBuilder_.buildMarkdown(sessionRecorder_.current(), diagnosisHistory_);
const QString outPath = QDir::currentPath() + "/reports/replay-report-" +
                        QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".md";
QFile f(outPath);
if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
  f.write(report.toUtf8());
  appendLog(QString("复盘报告已导出: %1").arg(outPath));
}
```

- [ ] **Step 4: Run one-shot verification (no repeated review cycle)**

Run: `cmake --build build --config Debug && ctest -C Debug --output-on-failure`  
Manual: `build/Debug/ibmmq_debug_assistant.exe` and export one report  
Expected: build/test pass; report file generated and readable

- [ ] **Step 5: Commit**

Run: `git add src/app/MainWindow.cpp README.md && git commit -m "feat: add replay report export and usage docs"`

## Self-Review

- **Spec coverage:** Covered A (rules, scoring, evidence, actions) and C (session record, read-only replay, report export) with dedicated tasks.
- **Placeholder scan:** No TODO/TBD placeholders; every coding step includes concrete snippets and commands.
- **Type consistency:** `DiagnosisContext/DiagnosisResult`, `SessionData/SessionEvent`, and replay/report types are introduced before integration tasks and reused consistently.
