#include "mq/MessageFormatter.h"

#include <iostream>

int main() {
  QByteArray in;
  in.append(char(0x01));
  in.append(char(0x02));
  in.append('A');
  const QString gotHex = toHexPreview(in);
  if (gotHex != QString("01 02 41")) {
    std::cerr << "hex preview assertion failed, got: " << gotHex.toStdString() << std::endl;
    return 1;
  }
  if (!matchMessage("order-1001", "1001")) {
    std::cerr << "search positive assertion failed" << std::endl;
    return 1;
  }
  if (matchMessage("order-1001", "2002")) {
    std::cerr << "search negative assertion failed" << std::endl;
    return 1;
  }
  return 0;
}
