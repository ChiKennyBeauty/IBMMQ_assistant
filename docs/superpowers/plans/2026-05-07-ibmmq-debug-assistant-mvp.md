# IBMMQ Debug Assistant MVP Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Windows-first Qt Widgets desktop tool that can connect to IBM MQ, browse Queue/Channel objects, browse queue messages with next-page pagination and current-page search, and run connectivity diagnostics.

**Architecture:** A single Qt Widgets app with one background worker object handling all MQ calls via IBM MQ C++ classes. UI sends requests through signals/slots and renders DTO-like response structs. Keep parsing/search simple (text/HEX + in-page filter only) to maximize delivery speed.

**Tech Stack:** C++17, Qt 5.14.2 (Widgets, Core, Test), IBM MQ C++ classes (`imqi.hpp`), CMake, Qt Test, ctest

---

## File Structure

- Create: `CMakeLists.txt` - build app and tests
- Create: `src/main.cpp` - app entry
- Create: `src/app/MainWindow.h` - UI shell
- Create: `src/app/MainWindow.cpp` - UI wiring and event handling
- Create: `src/app/ConnectionConfig.h` - connection config model
- Create: `src/mq/MqTypes.h` - shared request/response/error structs
- Create: `src/mq/MqWorker.h` - background worker interface
- Create: `src/mq/MqWorker.cpp` - MQ connection/object/message/diagnostics logic
- Create: `src/mq/MessageFormatter.h` - text/HEX and in-page search helpers
- Create: `src/mq/MessageFormatter.cpp` - formatter implementation
- Create: `tests/test_message_formatter.cpp` - formatter/search unit tests
- Create: `tests/test_connection_config.cpp` - config serialization tests
- Create: `README.md` - run/build basics and MVP scope

### Task 1: Bootstrap Build + App Shell

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/main.cpp`
- Create: `src/app/MainWindow.h`
- Create: `src/app/MainWindow.cpp`
- Test: `ctest` smoke (empty for now)

- [ ] **Step 1: Write the failing build setup test (configure/build should fail before files exist)**

Run: `cmake -S . -B build`
Expected: FAIL with missing `CMakeLists.txt`

- [ ] **Step 2: Add minimal CMake project and app target**

```cmake
cmake_minimum_required(VERSION 3.16)
project(IBMMQDebugAssistant LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Widgets Test)
add_executable(ibmmq_debug_assistant
    src/main.cpp
    src/app/MainWindow.cpp
)
target_include_directories(ibmmq_debug_assistant PRIVATE src)
target_link_libraries(ibmmq_debug_assistant PRIVATE Qt5::Widgets)
enable_testing()
```

- [ ] **Step 3: Add minimal main window implementation**

```cpp
// src/main.cpp
#include <QApplication>
#include "app/MainWindow.h"
int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  return app.exec();
}
```

```cpp
// src/app/MainWindow.h
#pragma once
#include <QMainWindow>
class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);
};
```

- [ ] **Step 4: Run configure/build to verify pass**

Run: `cmake -S . -B build && cmake --build build -j`
Expected: PASS, executable created

- [ ] **Step 5: Commit**

Run: `git add CMakeLists.txt src/main.cpp src/app/MainWindow.h src/app/MainWindow.cpp && git commit -m "chore: bootstrap Qt app shell with CMake"`

### Task 2: Connection Config Model + Persistence

**Files:**
- Create: `src/app/ConnectionConfig.h`
- Modify: `src/app/MainWindow.cpp`
- Create: `tests/test_connection_config.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write failing config serialization test**

```cpp
// tests/test_connection_config.cpp
void test_round_trip() {
  ConnectionConfig in{"dev", "QM1", "127.0.0.1(1414)", "DEV.APP.SVRCONN", "app", "pwd"};
  auto json = in.toJson();
  auto out = ConnectionConfig::fromJson(json);
  QCOMPARE(out.queueManager, QString("QM1"));
}
```

- [ ] **Step 2: Run test to verify failure**

Run: `ctest --test-dir build -R connection_config -V`
Expected: FAIL with missing symbols/types

- [ ] **Step 3: Implement minimal config model**

```cpp
struct ConnectionConfig {
  QString name, queueManager, connName, channel, username, password;
  QJsonObject toJson() const;
  static ConnectionConfig fromJson(const QJsonObject&);
};
```

- [ ] **Step 4: Re-run targeted test**

Run: `cmake --build build -j && ctest --test-dir build -R connection_config -V`
Expected: PASS

- [ ] **Step 5: Commit**

Run: `git add src/app/ConnectionConfig.h src/app/MainWindow.cpp tests/test_connection_config.cpp CMakeLists.txt && git commit -m "feat: add connection config persistence model"`

### Task 3: MQ Types + Worker Skeleton (Background Thread)

**Files:**
- Create: `src/mq/MqTypes.h`
- Create: `src/mq/MqWorker.h`
- Create: `src/mq/MqWorker.cpp`
- Modify: `src/app/MainWindow.h`
- Modify: `src/app/MainWindow.cpp`

- [ ] **Step 1: Write failing compile-time usage in UI**

```cpp
// MainWindow.cpp (temporary call)
emit requestConnect(currentConfig);
```

- [ ] **Step 2: Build and verify fail**

Run: `cmake --build build -j`
Expected: FAIL with undefined `requestConnect` / worker types

- [ ] **Step 3: Add minimal worker interface and wire to QThread**

```cpp
struct MqError { int code; QString reasonText; QString where; };
struct ConnectResult { bool ok; MqError error; };

class MqWorker : public QObject {
  Q_OBJECT
public slots:
  void connectToQueueManager(ConnectionConfig cfg);
signals:
  void connected(ConnectResult result);
};
```

- [ ] **Step 4: Build and run app smoke**

Run: `cmake --build build -j`
Expected: PASS

- [ ] **Step 5: Commit**

Run: `git add src/mq/MqTypes.h src/mq/MqWorker.h src/mq/MqWorker.cpp src/app/MainWindow.h src/app/MainWindow.cpp && git commit -m "feat: add mq worker skeleton on background thread"`

### Task 4: Implement Connect + Connectivity Test

**Files:**
- Modify: `src/mq/MqWorker.cpp`
- Modify: `src/mq/MqWorker.h`
- Modify: `src/app/MainWindow.cpp`
- Modify: `README.md`

- [ ] **Step 1: Write failing diagnostic UI expectation**

```cpp
// pseudo-check in UI slot
// on connected(false) -> append "CONNECT FAILED" to log panel
```

- [ ] **Step 2: Run manual app check to confirm missing behavior**

Run: `cmake --build build -j && build\\ibmmq_debug_assistant.exe`
Expected: no real MQ connect behavior yet

- [ ] **Step 3: Implement minimal MQ connect and test action**

```cpp
// in worker
ImqQueueManager mgr;
mgr.setName(cfg.queueManager.toLocal8Bit().constData());
if (!mgr.connect()) { emit connected({false, {int(mgr.reasonCode()), "connect failed", "connect"}}); return; }
mgr.disconnect();
emit connected({true, {}});
```

- [ ] **Step 4: Verify with reachable and unreachable config**

Run: launch app, test one valid and one invalid connection profile
Expected: log clearly shows success/failure with code

- [ ] **Step 5: Commit**

Run: `git add src/mq/MqWorker.cpp src/mq/MqWorker.h src/app/MainWindow.cpp README.md && git commit -m "feat: implement mq connection and connectivity diagnostics"`

### Task 5: Queue/Channel List Read-Only Browser

**Files:**
- Modify: `src/mq/MqTypes.h`
- Modify: `src/mq/MqWorker.h`
- Modify: `src/mq/MqWorker.cpp`
- Modify: `src/app/MainWindow.cpp`

- [ ] **Step 1: Write failing UI expectation for list refresh**

```cpp
// expected model sections: "Queues" and "Channels"
QCOMPARE(treeRoot->childCount(), 2);
```

- [ ] **Step 2: Verify fail**

Run: `cmake --build build -j`
Expected: FAIL / missing list response integration

- [ ] **Step 3: Implement list DTO and worker read operations**

```cpp
struct ObjectItem { QString type; QString name; QMap<QString, QString> attrs; };
struct ObjectListResult { bool ok; QVector<ObjectItem> items; MqError error; };
```

Use MQ classes to fetch queue/channel names and basic attributes, then emit result to UI.

- [ ] **Step 4: Manual verify object tree refresh**

Run: app -> connect -> click refresh
Expected: Queue + Channel nodes populate, errors shown in log if denied

- [ ] **Step 5: Commit**

Run: `git add src/mq/MqTypes.h src/mq/MqWorker.h src/mq/MqWorker.cpp src/app/MainWindow.cpp && git commit -m "feat: add read-only queue and channel browser"`

### Task 6: Message Browse (Next Page + Current Page Search)

**Files:**
- Create: `src/mq/MessageFormatter.h`
- Create: `src/mq/MessageFormatter.cpp`
- Modify: `src/mq/MqTypes.h`
- Modify: `src/mq/MqWorker.cpp`
- Modify: `src/app/MainWindow.cpp`
- Create: `tests/test_message_formatter.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write failing formatter/search tests**

```cpp
void test_hex_preview() {
  QByteArray in("\x01\x02A", 3);
  QCOMPARE(toHexPreview(in), QString("01 02 41"));
}
void test_in_page_search() {
  QVERIFY(matchMessage("order-1001", "1001"));
}
```

- [ ] **Step 2: Run tests to verify fail**

Run: `ctest --test-dir build -R message_formatter -V`
Expected: FAIL missing formatter functions

- [ ] **Step 3: Implement formatter + message browse response**

```cpp
struct MessageItem { QString textPreview; QString hexPreview; };
struct BrowseResult { bool ok; QVector<MessageItem> page; bool hasNext; MqError error; };
```

Implement MQ browse-first / browse-next flow and UI-side current-page keyword filter.

- [ ] **Step 4: Re-run tests + manual browse**

Run: `cmake --build build -j && ctest --test-dir build -R message_formatter -V`
Expected: tests PASS; app shows next-page and filter behavior

- [ ] **Step 5: Commit**

Run: `git add src/mq/MessageFormatter.h src/mq/MessageFormatter.cpp src/mq/MqTypes.h src/mq/MqWorker.cpp src/app/MainWindow.cpp tests/test_message_formatter.cpp CMakeLists.txt && git commit -m "feat: implement message browse with next-page and in-page search"`

### Task 7: Error UX + Logging + MVP Hardening

**Files:**
- Modify: `src/app/MainWindow.cpp`
- Modify: `src/mq/MqTypes.h`
- Modify: `README.md`

- [ ] **Step 1: Write failing behavior check list**

```text
1) failed connect shows code/reason/where
2) failed refresh keeps app responsive
3) failed message browse does not crash
```

- [ ] **Step 2: Reproduce current gaps manually**

Run: app with invalid config and invalid queue names
Expected: identify unclear error messages

- [ ] **Step 3: Implement unified error rendering**

```cpp
QString formatError(const MqError& e) {
  return QString("[%1] code=%2 reason=%3").arg(e.where).arg(e.code).arg(e.reasonText);
}
```

- [ ] **Step 4: Run final MVP verification**

Run: `cmake --build build -j && ctest --test-dir build -V`
Expected: PASS tests; manual checklist complete

- [ ] **Step 5: Commit**

Run: `git add src/app/MainWindow.cpp src/mq/MqTypes.h README.md && git commit -m "chore: polish error reporting and stabilize mvp workflows"`

## Self-Review

- **Spec coverage:** All MVP requirements are mapped: connection management (Task 2/4), queue-channel browse (Task 5), message browse/search (Task 6), diagnostics (Task 4/7).
- **Placeholder scan:** No TODO/TBD placeholders; each task includes concrete files, commands, and expected outcomes.
- **Type consistency:** `ConnectionConfig`, `MqError`, list and browse result structs are introduced before use and reused consistently.
