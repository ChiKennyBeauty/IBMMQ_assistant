#pragma once

#include <QByteArray>
#include <QString>

QString toHexPreview(const QByteArray &data);
QString toTextPreview(const QByteArray &data);
bool matchMessage(const QString &text, const QString &keyword);
