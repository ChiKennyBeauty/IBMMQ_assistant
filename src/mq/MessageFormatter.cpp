#include "mq/MessageFormatter.h"

QString toHexPreview(const QByteArray &data) {
  QString out;
  out.reserve(data.size() * 3);
  for (int i = 0; i < data.size(); ++i) {
    if (i > 0) {
      out += ' ';
    }
    out += QString("%1").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
  }
  return out;
}

QString toTextPreview(const QByteArray &data) {
  const QString utf8 = QString::fromUtf8(data.constData(), data.size());
  if (!utf8.isEmpty()) {
    return utf8;
  }
  return QString::fromLocal8Bit(data.constData(), data.size());
}

bool matchMessage(const QString &text, const QString &keyword) {
  if (keyword.trimmed().isEmpty()) {
    return true;
  }
  return text.contains(keyword, Qt::CaseInsensitive);
}
