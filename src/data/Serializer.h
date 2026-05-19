#pragma once

#include "DrawObject.h"
#include "PacketType.h"
#include <QByteArray>

namespace Serializer {

QByteArray                              serialize(const DrawObject &obj);
std::shared_ptr<DrawObject>             deserialize(const QByteArray &data);
void                                    saveToFile(const QString &path,
                                        const QVector<std::shared_ptr<DrawObject>> &objects);
QVector<std::shared_ptr<DrawObject> >   loadFromFile(const QString &path);
}
