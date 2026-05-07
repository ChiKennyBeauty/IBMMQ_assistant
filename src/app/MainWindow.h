#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QThread>

#include "app/ConnectionConfig.h"
#include "diagnosis/DiagnosisRuleEngine.h"
#include "mq/MqTypes.h"
#include "session/ReplayService.h"
#include "session/SessionRecorder.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

  ~MainWindow() override;

signals:
  void requestConnect(ConnectionConfig cfg);
  void requestListObjects(ConnectionConfig cfg);
  void requestBrowseMessages(ConnectionConfig cfg, QString queueName, int offset, int pageSize);
  void requestSendMessage(ConnectionConfig cfg, QString queueName, QString body);
  void requestReceiveMessage(ConnectionConfig cfg, QString queueName);

private:
  QThread workerThread_;
  ConnectionConfig currentConfig_;
  QTreeWidget *objectTree_ = nullptr;
  QListWidget *messageList_ = nullptr;
  QLineEdit *queueManagerEdit_ = nullptr;
  QLineEdit *channelEdit_ = nullptr;
  QLineEdit *connNameEdit_ = nullptr;
  QLineEdit *queueNameEdit_ = nullptr;
  QLineEdit *sendQueueEdit_ = nullptr;
  QLineEdit *messageInputEdit_ = nullptr;
  QLineEdit *searchEdit_ = nullptr;
  QPlainTextEdit *logBox_ = nullptr;
  QPlainTextEdit *diagnosisBox_ = nullptr;
  QPushButton *connectButton_ = nullptr;
  QPushButton *refreshButton_ = nullptr;
  QPushButton *browseButton_ = nullptr;
  QPushButton *receiveButton_ = nullptr;
  QPushButton *nextButton_ = nullptr;
  QPushButton *replayButton_ = nullptr;
  QPushButton *sendButton_ = nullptr;
  QPushButton *themeToggleButton_ = nullptr;
  bool isDarkTheme_ = false;
  bool hasMorePages_ = false;
  int browseOffset_ = 0;
  QVector<MessageItem> currentPage_;
  DiagnosisRuleEngine diagnosisEngine_;
  SessionRecorder sessionRecorder_;
  ReplayService replayService_;
  QVector<DiagnosisResult> diagnosisHistory_;

  void appendLog(const QString &message);
  void appendDiagnosis(const DiagnosisResult &diagnosis, const QString &scene, const MqError &error);
  void recordSessionEvent(const QString &eventType, bool ok, const MqError &error,
                          const QString &queueName = QString(), int itemCount = 0);
  void runReplay();
  void setBusy(bool busy);
  bool validateConnectionInputs() const;
  void applyTheme();
  QString buildLightThemeStyleSheet() const;
  QString buildDarkThemeStyleSheet() const;

  void onConnected(const ConnectResult &result);
  void onObjectsListed(const ObjectListResult &result);
  void onMessagesBrowsed(const BrowseResult &result);
  void onMessageSent(const SendResult &result);
  void onMessageReceived(const ReceiveResult &result);
};
