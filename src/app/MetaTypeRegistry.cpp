#include "app/MetaTypeRegistry.h"

#include <QMetaType>

#include "app/ConnectionConfig.h"
#include "mq/MqTypes.h"

void registerAppMetaTypes() {
  qRegisterMetaType<ConnectionConfig>("ConnectionConfig");
  qRegisterMetaType<ConnectResult>("ConnectResult");
  qRegisterMetaType<ObjectListResult>("ObjectListResult");
  qRegisterMetaType<BrowseResult>("BrowseResult");
  qRegisterMetaType<SendResult>("SendResult");
  qRegisterMetaType<ReceiveResult>("ReceiveResult");
}
