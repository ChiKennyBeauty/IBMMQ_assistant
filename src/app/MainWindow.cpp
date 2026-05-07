#include "app/MainWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QFormLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "mq/MessageFormatter.h"
#include "mq/MqWorker.h"
#include "report/ReportBuilder.h"

namespace {
QString formatError(const MqError &error) {
  return QString("[%1] code=%2 %3").arg(error.where).arg(error.code).arg(error.reasonText);
}

QString buildCommonStyleSheet() {
  return QString(
      "QWidget { font-size: 13px; }"
      "QGroupBox {"
      "  font-weight: 600;"
      "  border-radius: 10px;"
      "  margin-top: 10px;"
      "  padding-top: 10px;"
      "}"
      "QGroupBox::title {"
      "  subcontrol-origin: margin;"
      "  left: 10px;"
      "  padding: 0 4px;"
      "}"
      "QPushButton {"
      "  border: none;"
      "  border-radius: 7px;"
      "  padding: 6px 12px;"
      "}"
      "QPushButton:disabled {"
      "  background: #9CA3AF;"
      "  color: #F3F4F6;"
      "}"
      "QLineEdit {"
      "  border-radius: 7px;"
      "  padding: 6px 8px;"
      "}"
      "QTreeWidget, QListWidget, QPlainTextEdit {"
      "  border-radius: 8px;"
      "}"
      "QHeaderView::section {"
      "  border: none;"
      "  border-right: 1px solid #E5E7EB;"
      "  padding: 6px;"
      "  font-weight: 600;"
      "}");
}
} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("IBMMQ \u8c03\u8bd5\u52a9\u624b");
  resize(1180, 760);

  auto *worker = new MqWorker();
  worker->moveToThread(&workerThread_);
  connect(&workerThread_, &QThread::finished, worker, &QObject::deleteLater);
  connect(this, &MainWindow::requestConnect, worker, &MqWorker::connectToQueueManager);
  connect(this, &MainWindow::requestListObjects, worker, &MqWorker::listObjects);
  connect(this, &MainWindow::requestBrowseMessages, worker, &MqWorker::browseMessages);
  connect(this, &MainWindow::requestSendMessage, worker, &MqWorker::sendMessage);
  connect(this, &MainWindow::requestReceiveMessage, worker, &MqWorker::receiveMessage);
  connect(worker, &MqWorker::connected, this, &MainWindow::onConnected);
  connect(worker, &MqWorker::objectsListed, this, &MainWindow::onObjectsListed);
  connect(worker, &MqWorker::messagesBrowsed, this, &MainWindow::onMessagesBrowsed);
  connect(worker, &MqWorker::messageSent, this, &MainWindow::onMessageSent);
  connect(worker, &MqWorker::messageReceived, this, &MainWindow::onMessageReceived);
  workerThread_.start();

  auto *root = new QWidget(this);
  auto *layout = new QVBoxLayout(root);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setSpacing(10);
  auto *title = new QLabel("IBMMQ \u8c03\u8bd5\u52a9\u624b", root);
  title->setStyleSheet("font-size: 18px; font-weight: 600; padding: 4px 0;");
  auto *buttonRow = new QWidget(root);
  auto *buttonLayout = new QHBoxLayout(buttonRow);
  buttonLayout->setSpacing(8);
  connectButton_ = new QPushButton("\u6d4b\u8bd5\u8fde\u63a5", buttonRow);
  refreshButton_ = new QPushButton("\u5237\u65b0\u5bf9\u8c61", buttonRow);
  browseButton_ = new QPushButton("\u6d4f\u89c8\u6d88\u606f", buttonRow);
  receiveButton_ = new QPushButton("\u6536\u53d6\u4e00\u6761", buttonRow);
  nextButton_ = new QPushButton("\u4e0b\u4e00\u9875", buttonRow);
  replayButton_ = new QPushButton("\u56de\u653e\u4f1a\u8bdd", buttonRow);
  themeToggleButton_ = new QPushButton("\u5207\u6362\u5230\u6697\u8272", buttonRow);
  auto *exportReportButton = new QPushButton("\u5bfc\u51fa\u62a5\u544a", buttonRow);
  connectButton_->setMinimumHeight(34);
  refreshButton_->setMinimumHeight(34);
  browseButton_->setMinimumHeight(34);
  receiveButton_->setMinimumHeight(34);
  nextButton_->setMinimumHeight(34);
  replayButton_->setMinimumHeight(34);
  themeToggleButton_->setMinimumHeight(34);
  exportReportButton->setMinimumHeight(34);
  nextButton_->setEnabled(false);
  buttonLayout->addWidget(connectButton_);
  buttonLayout->addWidget(refreshButton_);
  buttonLayout->addWidget(browseButton_);
  buttonLayout->addWidget(receiveButton_);
  buttonLayout->addWidget(nextButton_);
  buttonLayout->addWidget(replayButton_);
  buttonLayout->addWidget(themeToggleButton_);
  buttonLayout->addWidget(exportReportButton);
  buttonLayout->addStretch();
  buttonLayout->setContentsMargins(0, 0, 0, 0);

  auto *contentRow = new QWidget(root);
  auto *contentLayout = new QHBoxLayout(contentRow);
  contentLayout->setContentsMargins(0, 0, 0, 0);
  contentLayout->setSpacing(10);

  auto *leftPanel = new QWidget(contentRow);
  auto *leftLayout = new QVBoxLayout(leftPanel);
  leftLayout->setContentsMargins(0, 0, 0, 0);
  leftLayout->setSpacing(8);

  auto *configPanel = new QGroupBox("\u8fde\u63a5\u53c2\u6570", contentRow);
  configPanel->setMinimumWidth(300);
  auto *configPanelLayout = new QVBoxLayout(configPanel);
  auto *configLayout = new QFormLayout();
  queueManagerEdit_ = new QLineEdit(configPanel);
  channelEdit_ = new QLineEdit(configPanel);
  connNameEdit_ = new QLineEdit(configPanel);
  queueNameEdit_ = new QLineEdit(configPanel);
  configLayout->addRow("\u961f\u5217\u7ba1\u7406\u5668", queueManagerEdit_);
  configLayout->addRow("\u901a\u9053", channelEdit_);
  configLayout->addRow("\u8fde\u63a5\u5730\u5740", connNameEdit_);
  configLayout->addRow("\u6d4f\u89c8\u961f\u5217", queueNameEdit_);
  configPanelLayout->addLayout(configLayout);
  configPanelLayout->addStretch();

  objectTree_ = new QTreeWidget(root);
  objectTree_->setHeaderLabels({"\u7c7b\u578b", "\u540d\u79f0", "\u4fe1\u606f"});
  objectTree_->setAlternatingRowColors(true);

  auto *messageRow = new QWidget(leftPanel);
  auto *messageLayout = new QVBoxLayout(messageRow);
  auto *searchRow = new QWidget(messageRow);
  auto *searchLayout = new QHBoxLayout(searchRow);
  searchEdit_ = new QLineEdit(searchRow);
  searchEdit_->setPlaceholderText("\u8fc7\u6ee4\u5f53\u524d\u9875...");
  searchLayout->addWidget(new QLabel("\u641c\u7d22", searchRow));
  searchLayout->addWidget(searchEdit_);
  searchLayout->setContentsMargins(0, 0, 0, 0);
  messageList_ = new QListWidget(messageRow);
  messageList_->setAlternatingRowColors(true);
  messageLayout->addWidget(searchRow);
  messageLayout->addWidget(messageList_);
  messageLayout->setContentsMargins(0, 0, 0, 0);

  logBox_ = new QPlainTextEdit(leftPanel);
  logBox_->setReadOnly(true);
  logBox_->setObjectName("logBox");
  diagnosisBox_ = new QPlainTextEdit(leftPanel);
  diagnosisBox_->setReadOnly(true);
  diagnosisBox_->setObjectName("diagnosisBox");
  diagnosisBox_->setPlaceholderText("故障诊断建议将显示在这里");

  auto *sendPanel = new QGroupBox("\u6d88\u606f\u53d1\u9001", root);
  auto *sendPanelLayout = new QVBoxLayout(sendPanel);
  auto *sendQueueRow = new QWidget(sendPanel);
  auto *sendQueueLayout = new QHBoxLayout(sendQueueRow);
  sendQueueLayout->setContentsMargins(0, 0, 0, 0);
  sendQueueLayout->setSpacing(8);
  sendQueueEdit_ = new QLineEdit(sendQueueRow);
  sendQueueLayout->addWidget(new QLabel("\u53d1\u9001\u961f\u5217", sendQueueRow));
  sendQueueLayout->addWidget(sendQueueEdit_, 1);

  auto *sendLayout = new QHBoxLayout();
  messageInputEdit_ = new QLineEdit(sendPanel);
  sendButton_ = new QPushButton("\u53d1\u9001\u6d88\u606f", sendPanel);
  sendButton_->setMinimumHeight(34);
  messageInputEdit_->setPlaceholderText("\u8bf7\u8f93\u5165\u8981\u53d1\u9001\u7684\u6d88\u606f\u5185\u5bb9");
  sendLayout->addWidget(new QLabel("\u53d1\u9001\u5185\u5bb9", sendPanel));
  sendLayout->addWidget(messageInputEdit_, 1);
  sendLayout->addWidget(sendButton_);
  sendPanelLayout->addWidget(sendQueueRow);
  sendPanelLayout->addLayout(sendLayout);

  leftLayout->addWidget(objectTree_, 1);
  leftLayout->addWidget(messageRow, 1);
  leftLayout->addWidget(logBox_, 1);
  leftLayout->addWidget(diagnosisBox_, 1);

  contentLayout->addWidget(leftPanel, 1);
  contentLayout->addWidget(configPanel);

  layout->addWidget(title);
  layout->addWidget(buttonRow);
  layout->addWidget(contentRow, 1);
  layout->addWidget(sendPanel);
  setCentralWidget(root);
  sessionRecorder_.startSession("dev");

  currentConfig_.name = "default";
  currentConfig_.queueManager = "QM1";
  currentConfig_.channel = "SYSTEM.DEF.SVRCONN";
  currentConfig_.connName = "127.0.0.1(1414)";
  const QString defaultQueueName = "SYSTEM.DEFAULT.LOCAL.QUEUE";

  if (queueManagerEdit_) {
    queueManagerEdit_->setText(currentConfig_.queueManager);
  }
  if (channelEdit_) {
    channelEdit_->setText(currentConfig_.channel);
  }
  if (connNameEdit_) {
    connNameEdit_->setText(currentConfig_.connName);
  }
  if (queueNameEdit_) {
    queueNameEdit_->setText(defaultQueueName);
  }
  if (sendQueueEdit_) {
    sendQueueEdit_->setText(defaultQueueName);
  }
  auto readConfigFromUi = [this]() {
    if (queueManagerEdit_) {
      currentConfig_.queueManager = queueManagerEdit_->text().trimmed();
    }
    if (channelEdit_) {
      currentConfig_.channel = channelEdit_->text().trimmed();
    }
    if (connNameEdit_) {
      currentConfig_.connName = connNameEdit_->text().trimmed();
    }
  };

  connect(connectButton_, &QPushButton::clicked, this, [this, readConfigFromUi]() {
    readConfigFromUi();
    if (!validateConnectionInputs()) {
      return;
    }
    setBusy(true);
    emit requestConnect(currentConfig_);
  });

  connect(refreshButton_, &QPushButton::clicked, this, [this, readConfigFromUi]() {
    readConfigFromUi();
    if (!validateConnectionInputs()) {
      return;
    }
    setBusy(true);
    emit requestListObjects(currentConfig_);
  });

  connect(browseButton_, &QPushButton::clicked, this, [this, readConfigFromUi]() {
    readConfigFromUi();
    if (!validateConnectionInputs()) {
      return;
    }
    browseOffset_ = 0;
    const QString queueName = queueNameEdit_ ? queueNameEdit_->text().trimmed() : QString();
    if (queueName.isEmpty()) {
      appendLog("\u6d4f\u89c8\u5931\u8d25: \u6d4f\u89c8\u961f\u5217\u4e0d\u80fd\u4e3a\u7a7a");
      return;
    }
    hasMorePages_ = false;
    if (nextButton_) {
      nextButton_->setEnabled(false);
    }
    setBusy(true);
    emit requestBrowseMessages(currentConfig_, queueName, browseOffset_, 10);
  });

  connect(nextButton_, &QPushButton::clicked, this, [this, readConfigFromUi]() {
    readConfigFromUi();
    if (!validateConnectionInputs()) {
      return;
    }
    const QString queueName = queueNameEdit_ ? queueNameEdit_->text().trimmed() : QString();
    if (queueName.isEmpty()) {
      appendLog("\u6d4f\u89c8\u5931\u8d25: \u6d4f\u89c8\u961f\u5217\u4e0d\u80fd\u4e3a\u7a7a");
      return;
    }
    setBusy(true);
    emit requestBrowseMessages(currentConfig_, queueName, browseOffset_, 10);
  });

  connect(receiveButton_, &QPushButton::clicked, this, [this, readConfigFromUi]() {
    readConfigFromUi();
    if (!validateConnectionInputs()) {
      return;
    }
    const QString queueName = queueNameEdit_ ? queueNameEdit_->text().trimmed() : QString();
    if (queueName.isEmpty()) {
      appendLog("\u6536\u53d6\u5931\u8d25: \u6d4f\u89c8\u961f\u5217\u4e0d\u80fd\u4e3a\u7a7a");
      return;
    }
    setBusy(true);
    emit requestReceiveMessage(currentConfig_, queueName);
  });

  connect(sendButton_, &QPushButton::clicked, this, [this, readConfigFromUi]() {
    readConfigFromUi();
    if (!validateConnectionInputs()) {
      return;
    }
    const QString queueName = sendQueueEdit_ ? sendQueueEdit_->text().trimmed() : QString();
    if (queueName.isEmpty()) {
      appendLog("\u53d1\u9001\u5931\u8d25: \u53d1\u9001\u961f\u5217\u4e0d\u80fd\u4e3a\u7a7a");
      return;
    }
    const QString body = messageInputEdit_ ? messageInputEdit_->text() : QString();
    if (body.trimmed().isEmpty()) {
      appendLog("\u53d1\u9001\u5931\u8d25: \u6d88\u606f\u5185\u5bb9\u4e0d\u80fd\u4e3a\u7a7a");
      return;
    }
    setBusy(true);
    emit requestSendMessage(currentConfig_, queueName, body);
  });

  connect(replayButton_, &QPushButton::clicked, this, [this]() { runReplay(); });
  connect(themeToggleButton_, &QPushButton::clicked, this, [this]() {
    isDarkTheme_ = !isDarkTheme_;
    applyTheme();
  });

  connect(exportReportButton, &QPushButton::clicked, this, [this]() {
    ReportBuilder reportBuilder;
    const QString markdown =
        reportBuilder.buildMarkdown(sessionRecorder_.current(), diagnosisHistory_);
    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString fileName = QString("replay-report-%1.md").arg(stamp);
    QDir reportsDir("reports");
    if (!reportsDir.exists() && !reportsDir.mkpath(".")) {
      appendLog(QString("\u62a5\u544a\u5bfc\u51fa\u5931\u8d25: \u65e0\u6cd5\u521b\u5efa\u76ee\u5f55 %1")
                    .arg(reportsDir.absolutePath()));
      return;
    }
    const QString filePath = reportsDir.filePath(fileName);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
      appendLog(QString("\u62a5\u544a\u5bfc\u51fa\u5931\u8d25: \u65e0\u6cd5\u5199\u5165 %1")
                    .arg(QFileInfo(filePath).absoluteFilePath()));
      return;
    }
    file.write(markdown.toUtf8());
    file.close();
    appendLog(QString("\u62a5\u544a\u5bfc\u51fa\u6210\u529f: %1")
                  .arg(QFileInfo(filePath).absoluteFilePath()));
  });

  connect(searchEdit_, &QLineEdit::textChanged, this, [this]() {
    if (!messageList_) {
      return;
    }
    messageList_->clear();
    const QString keyword = searchEdit_ ? searchEdit_->text() : QString();
    for (const auto &item : currentPage_) {
      if (matchMessage(item.textPreview, keyword) || matchMessage(item.hexPreview, keyword)) {
        messageList_->addItem(item.textPreview);
      }
    }
  });

  applyTheme();
}

MainWindow::~MainWindow() {
  workerThread_.quit();
  workerThread_.wait();
}

void MainWindow::onConnected(const ConnectResult &result) {
  setBusy(false);
  recordSessionEvent("onConnected", result.ok, result.error);

  if (result.ok) {
    appendLog("\u8fde\u63a5\u6210\u529f");
    return;
  }

  appendLog(QString("\u8fde\u63a5\u5931\u8d25: %1").arg(formatError(result.error)));
  DiagnosisContext context;
  context.scene = "connect";
  context.errorCode = result.error.code;
  context.errorText = result.error.reasonText;
  context.attrs.insert("where", result.error.where);
  appendDiagnosis(diagnosisEngine_.diagnose(context), context.scene, result.error);
}

void MainWindow::onObjectsListed(const ObjectListResult &result) {
  setBusy(false);
  recordSessionEvent("onObjectsListed", result.ok, result.error, QString(), result.items.size());
  if (!objectTree_) {
    return;
  }

  objectTree_->clear();
  if (!result.ok) {
    appendLog(QString("\u5237\u65b0\u5931\u8d25: %1").arg(formatError(result.error)));
    DiagnosisContext context;
    context.scene = "listObjects";
    context.errorCode = result.error.code;
    context.errorText = result.error.reasonText;
    context.attrs.insert("where", result.error.where);
    appendDiagnosis(diagnosisEngine_.diagnose(context), context.scene, result.error);
    return;
  }

  for (const auto &item : result.items) {
    QString info;
    for (auto it = item.attrs.cbegin(); it != item.attrs.cend(); ++it) {
      if (!info.isEmpty()) {
        info += "; ";
      }
      info += QString("%1=%2").arg(it.key(), it.value());
    }
    auto *node = new QTreeWidgetItem({item.type, item.name, info});
    objectTree_->addTopLevelItem(node);
  }
  appendLog(QString("\u5237\u65b0\u6210\u529f: %1 \u4e2a\u5bf9\u8c61").arg(result.items.size()));
}

void MainWindow::onMessagesBrowsed(const BrowseResult &result) {
  setBusy(false);
  const QString browseQueueName = queueNameEdit_ ? queueNameEdit_->text().trimmed() : QString();
  recordSessionEvent("onMessagesBrowsed", result.ok, result.error, browseQueueName, result.page.size());
  if (!messageList_) {
    return;
  }

  if (!result.ok) {
    appendLog(QString("\u6d4f\u89c8\u5931\u8d25: %1").arg(formatError(result.error)));
    DiagnosisContext context;
    context.scene = "browse";
    context.errorCode = result.error.code;
    context.errorText = result.error.reasonText;
    context.attrs.insert("where", result.error.where);
    context.attrs.insert("queueName", browseQueueName);
    appendDiagnosis(diagnosisEngine_.diagnose(context), context.scene, result.error);
    return;
  }

  currentPage_ = result.page;
  browseOffset_ = result.nextOffset;
  messageList_->clear();
  const QString keyword = searchEdit_ ? searchEdit_->text() : QString();
  for (const auto &item : currentPage_) {
    if (matchMessage(item.textPreview, keyword) || matchMessage(item.hexPreview, keyword)) {
      messageList_->addItem(item.textPreview);
    }
  }
  hasMorePages_ = result.hasMore;
  if (nextButton_) {
    nextButton_->setEnabled(result.hasMore);
  }
  appendLog(
      QString("\u6d4f\u89c8\u6210\u529f: %1 \u6761\u6d88\u606f\uff0c\u66f4\u591a=%2")
          .arg(result.page.size())
          .arg(result.hasMore ? "\u662f" : "\u5426"));
}

void MainWindow::onMessageSent(const SendResult &result) {
  setBusy(false);
  recordSessionEvent("onMessageSent", result.ok, result.error, result.queueName);
  if (!result.ok) {
    appendLog(QString("\u53d1\u9001\u5931\u8d25: %1").arg(formatError(result.error)));
    DiagnosisContext context;
    context.scene = "send";
    context.errorCode = result.error.code;
    context.errorText = result.error.reasonText;
    context.attrs.insert("where", result.error.where);
    context.attrs.insert("queueName", result.queueName);
    appendDiagnosis(diagnosisEngine_.diagnose(context), context.scene, result.error);
    return;
  }
  appendLog(
      QString("\u53d1\u9001\u6210\u529f: \u961f\u5217=%1, \u5b57\u8282\u6570=%2")
          .arg(result.queueName)
          .arg(result.bytes));
}

void MainWindow::onMessageReceived(const ReceiveResult &result) {
  setBusy(false);
  recordSessionEvent("onMessageReceived", result.ok, result.error, result.queueName,
                     result.hasMessage ? 1 : 0);
  if (!messageList_) {
    return;
  }
  if (!result.ok) {
    appendLog(QString("\u6536\u53d6\u5931\u8d25: %1").arg(formatError(result.error)));
    DiagnosisContext context;
    context.scene = "receive";
    context.errorCode = result.error.code;
    context.errorText = result.error.reasonText;
    context.attrs.insert("where", result.error.where);
    context.attrs.insert("queueName", result.queueName);
    appendDiagnosis(diagnosisEngine_.diagnose(context), context.scene, result.error);
    return;
  }
  if (!result.hasMessage) {
    appendLog(QString("\u6536\u53d6\u6210\u529f: \u961f\u5217 %1 \u6682\u65e0\u6d88\u606f").arg(result.queueName));
    return;
  }
  messageList_->insertItem(0, result.message.textPreview);
  appendLog(QString("\u6536\u53d6\u6210\u529f: \u961f\u5217=%1").arg(result.queueName));
}

void MainWindow::appendLog(const QString &message) {
  if (!logBox_) {
    return;
  }
  const QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
  logBox_->appendPlainText(QString("[%1] %2").arg(ts, message));
}

void MainWindow::appendDiagnosis(const DiagnosisResult &diagnosis, const QString &scene,
                                 const MqError &error) {
  diagnosisHistory_.append(diagnosis);
  if (!diagnosisBox_) {
    return;
  }
  const QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
  const QString faultType = diagnosis.faultType.isEmpty() ? "Unknown" : diagnosis.faultType;
  diagnosisBox_->appendPlainText(
      QString("[%1] scene=%2 faultType=%3 confidence=%4").arg(ts, scene, faultType).arg(
          diagnosis.confidence));
  if (!error.reasonText.isEmpty()) {
    diagnosisBox_->appendPlainText(QString("  error=%1 (code=%2)").arg(error.reasonText).arg(error.code));
  }
  if (!diagnosis.actions.isEmpty()) {
    for (const auto &action : diagnosis.actions) {
      diagnosisBox_->appendPlainText(QString("  - %1").arg(action));
    }
  }
  diagnosisBox_->appendPlainText(QString());
}

void MainWindow::recordSessionEvent(const QString &eventType, bool ok, const MqError &error,
                                    const QString &queueName, int itemCount) {
  QJsonObject snapshot;
  snapshot["queueManager"] = currentConfig_.queueManager;
  snapshot["channel"] = currentConfig_.channel;
  snapshot["connName"] = currentConfig_.connName;
  if (!queueName.isEmpty()) {
    snapshot["queueName"] = queueName;
  }
  if (itemCount > 0) {
    snapshot["itemCount"] = itemCount;
  }

  SessionEvent event;
  event.eventType = eventType;
  event.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
  event.inputSnapshot = snapshot;
  event.ok = ok;
  event.durationMs = 0;
  if (!ok) {
    event.errorCode = error.code;
    event.errorText = error.reasonText;
  }
  sessionRecorder_.recordEvent(event);
}

void MainWindow::runReplay() {
  const SessionData session = sessionRecorder_.current();
  const QVector<ReplayDelta> deltas = replayService_.replayReadOnly(session);
  appendLog(QString("会话回放: 只读模式 步骤=%1").arg(deltas.size()));
  if (deltas.isEmpty()) {
    appendLog("会话回放: 未找到 connect/list/browse 事件");
    return;
  }
  for (const ReplayDelta &delta : deltas) {
    appendLog(QString("回放 步骤-%1 | 历史=%2 | 当前=%3")
                  .arg(delta.step, delta.history, delta.current));
  }
}

void MainWindow::setBusy(bool busy) {
  if (connectButton_) {
    connectButton_->setEnabled(!busy);
  }
  if (refreshButton_) {
    refreshButton_->setEnabled(!busy);
  }
  if (browseButton_) {
    browseButton_->setEnabled(!busy);
  }
  if (receiveButton_) {
    receiveButton_->setEnabled(!busy);
  }
  if (sendButton_) {
    sendButton_->setEnabled(!busy);
  }
  if (nextButton_) {
    // "next page" is special: enabled only when there's more page data.
    nextButton_->setEnabled(!busy && hasMorePages_);
  }
}

bool MainWindow::validateConnectionInputs() const {
  if (!queueManagerEdit_ || !channelEdit_ || !connNameEdit_) {
    return true;
  }

  if (queueManagerEdit_->text().trimmed().isEmpty()) {
    const_cast<MainWindow *>(this)->appendLog("\u53c2\u6570\u9519\u8bef: \u961f\u5217\u7ba1\u7406\u5668\u4e0d\u80fd\u4e3a\u7a7a");
    return false;
  }
  if (channelEdit_->text().trimmed().isEmpty()) {
    const_cast<MainWindow *>(this)->appendLog("\u53c2\u6570\u9519\u8bef: \u901a\u9053\u4e0d\u80fd\u4e3a\u7a7a");
    return false;
  }

  const QString connName = connNameEdit_->text().trimmed();
  const QRegularExpression connPattern("^.+\\(\\d+\\)$");
  if (connName.isEmpty() || !connPattern.match(connName).hasMatch()) {
    const_cast<MainWindow *>(this)->appendLog(
        "\u53c2\u6570\u9519\u8bef: \u8fde\u63a5\u5730\u5740\u683c\u5f0f\u5e94\u4e3a host(port)\uff0c\u4f8b\u5982 127.0.0.1(1414)");
    return false;
  }
  return true;
}

void MainWindow::applyTheme() {
  if (!centralWidget()) {
    return;
  }
  centralWidget()->setStyleSheet(isDarkTheme_ ? buildDarkThemeStyleSheet() : buildLightThemeStyleSheet());
  if (themeToggleButton_) {
    themeToggleButton_->setText(isDarkTheme_ ? "切换到浅色" : "切换到暗色");
  }
}

QString MainWindow::buildLightThemeStyleSheet() const {
  return buildCommonStyleSheet() +
         "QWidget { background: #F5F7FB; color: #1F2937; }"
         "QGroupBox { border: 1px solid #D8DEE9; background: #F8FAFC; }"
         "QPushButton { background: #2563EB; color: #FFFFFF; }"
         "QPushButton:hover { background: #1D4ED8; }"
         "QPushButton:pressed { background: #1E40AF; }"
         "QLineEdit { border: 1px solid #CBD5E1; background: #FFFFFF; color: #111827; }"
         "QTreeWidget, QListWidget, QPlainTextEdit { border: 1px solid #D1D5DB; background: #FFFFFF; color: #111827; alternate-background-color: #F8FAFC; }"
         "QTreeWidget::item, QListWidget::item { background: transparent; color: #111827; }"
         "QTreeWidget::item:selected, QListWidget::item:selected { background: #DBEAFE; color: #111827; }"
         "QHeaderView::section { background: #F3F4F6; color: #374151; }";
}

QString MainWindow::buildDarkThemeStyleSheet() const {
  return buildCommonStyleSheet() +
         "QWidget { background: #0F172A; color: #E5E7EB; }"
         "QGroupBox { border: 1px solid #334155; background: #111827; }"
         "QPushButton { background: #3B82F6; color: #F8FAFC; }"
         "QPushButton:hover { background: #2563EB; }"
         "QPushButton:pressed { background: #1D4ED8; }"
         "QLineEdit { border: 1px solid #475569; background: #1F2937; color: #F3F4F6; }"
         "QLineEdit:focus { border: 1px solid #60A5FA; }"
         "QTreeWidget, QListWidget, QPlainTextEdit { border: 1px solid #334155; background: #111827; color: #E5E7EB; alternate-background-color: #0B1220; }"
         "QTreeWidget::item, QListWidget::item { background: transparent; color: #E5E7EB; }"
         "QTreeWidget::item:selected, QListWidget::item:selected { background: #1E3A8A; color: #F8FAFC; }"
         "QHeaderView::section { background: #1E293B; color: #CBD5E1; border-right: 1px solid #334155; }";
}
