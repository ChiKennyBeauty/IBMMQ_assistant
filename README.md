# IBMMQ 调试助手 (IBMMQ Debug Assistant)

A Qt5-based desktop tool for debugging IBM MQ queue managers: connect, browse,
send, and receive messages, with an embedded diagnosis rule engine and session
recording / replay.

## Features

- Connect to a queue manager and list/browse/send/receive messages
- Built-in diagnosis rule engine for common MQ errors
- Session recorder + read-only replay of `connect` / `list` / `browse` events
- **Export replay report** to Markdown from the current session and diagnosis
  history

## Export replay report

Click **导出报告** (Export Report) in the toolbar to generate a Markdown report
using `ReportBuilder::buildMarkdown(session, diagnosisHistory)`. The file is
written under the `reports/` directory (created automatically if missing) with
a timestamped name:

```
reports/replay-report-YYYYMMDD-HHMMSS.md
```

The absolute path of the generated file is appended to the application log.

## Build & test

```bash
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```
