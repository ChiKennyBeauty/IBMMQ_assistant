#include "app/MainWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QRegularExpression>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "mq/MessageFormatter.h"
#include "mq/MqWorker.h"

namespace {
QString formatError(const MqError &error) {
  return QString("[%1] code=%2 %3").arg(error.where).arg(error.code).arg(error.reasonText);
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
  auto *title = new QLabel("IBMMQ \u8c03\u8bd5\u52a9\u624b MVP", root);
  title->setStyleSheet("font-size: 18px; font-weight: 600; padding: 4px 0;");
  auto *buttonRow = new QWidget(root);
  auto *buttonLayout = new QHBoxLayout(buttonRow);
  buttonLayout->setSpacing(8);
  connectButton_ = new QPushButton("\u6d4b\u8bd5\u8fde\u63a5", buttonRow);
  refreshButton_ = new QPushButton("\u5237\u65b0\u5bf9\u8c61", buttonRow);
  browseButton_ = new QPushButton("\u6d4f\u89c8\u6d88\u606f", buttonRow);
  receiveButton_ = new QPushButton("\u6536\u53d6\u4e00\u6761", buttonRow);
  nextButton_ = new QPushButton("\u4e0b\u4e00\u9875", buttonRow);
  connectButton_->setMinimumHeight(34);
  refreshButton_->setMinimumHeight(34);
  browseButton_->setMinimumHeight(34);
  receiveButton_->setMinimumHeight(34);
  nextButton_->setMinimumHeight(34);
  nextButton_->setEnabled(false);
  buttonLayout->addWidget(connectButton_);
  buttonLayout->addWidget(refreshButton_);
  buttonLayout->addWidget(browseButton_);
  buttonLayout->addWidget(receiveButton_);
  buttonLayout->addWidget(nextButton_);
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

  root->setStyleSheet(
      "QWidget { font-size: 13px; }"
      "QGroupBox {"
      "  font-weight: 600;"
      "  border: 1px solid #D8DEE9;"
      "  border-radius: 8px;"
      "  margin-top: 10px;"
      "  padding-top: 8px;"
      "  background: #F8FAFC;"
      "}"
      "QGroupBox::title {"
      "  subcontrol-origin: margin;"
      "  left: 10px;"
      "  padding: 0 4px;"
      "}"
      "QPushButton {"
      "  background: #2563EB;"
      "  color: white;"
      "  border: none;"
      "  border-radius: 6px;"
      "  padding: 6px 12px;"
      "}"
      "QPushButton:hover { background: #1D4ED8; }"
      "QPushButton:pressed { background: #1E40AF; }"
      "QLineEdit {"
      "  border: 1px solid #CBD5E1;"
      "  border-radius: 6px;"
      "  padding: 6px 8px;"
      "  background: white;"
      "}"
      "QTreeWidget, QListWidget, QPlainTextEdit {"
      "  border: 1px solid #D1D5DB;"
      "  border-radius: 6px;"
      "  background: white;"
      "}"
      "QHeaderView::section {"
      "  background: #F3F4F6;"
      "  border: none;"
      "  border-right: 1px solid #E5E7EB;"
      "  padding: 6px;"
      "  font-weight: 600;"
      "}");

  leftLayout->addWidget(objectTree_, 1);
  leftLayout->addWidget(messageRow, 1);
  leftLayout->addWidget(logBox_, 1);

  contentLayout->addWidget(leftPanel, 1);
  contentLayout->addWidget(configPanel);

  layout->addWidget(title);
  layout->addWidget(buttonRow);
  layout->addWidget(contentRow, 1);
  layout->addWidget(sendPanel);
  setCentralWidget(root);

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
}

MainWindow::~MainWindow() {
  workerThread_.quit();
  workerThread_.wait();
}

void MainWindow::onConnected(const ConnectResult &result) {
  setBusy(false);

  if (result.ok) {
    appendLog("\u8fde\u63a5\u6210\u529f");
    return;
  }

  appendLog(QString("\u8fde\u63a5\u5931\u8d25: %1").arg(formatError(result.error)));
}

void MainWindow::onObjectsListed(const ObjectListResult &result) {
  setBusy(false);
  if (!objectTree_) {
    return;
  }

  objectTree_->clear();
  if (!result.ok) {
    appendLog(QString("\u5237\u65b0\u5931\u8d25: %1").arg(formatError(result.error)));
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
  if (!messageList_) {
    return;
  }

  if (!result.ok) {
    appendLog(QString("\u6d4f\u89c8\u5931\u8d25: %1").arg(formatError(result.error)));
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
  if (!result.ok) {
    appendLog(QString("\u53d1\u9001\u5931\u8d25: %1").arg(formatError(result.error)));
    return;
  }
  appendLog(
      QString("\u53d1\u9001\u6210\u529f: \u961f\u5217=%1, \u5b57\u8282\u6570=%2")
          .arg(result.queueName)
          .arg(result.bytes));
}

void MainWindow::onMessageReceived(const ReceiveResult &result) {
  setBusy(false);
  if (!messageList_) {
    return;
  }
  if (!result.ok) {
    appendLog(QString("\u6536\u53d6\u5931\u8d25: %1").arg(formatError(result.error)));
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
