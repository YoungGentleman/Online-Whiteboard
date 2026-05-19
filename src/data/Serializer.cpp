#include "Serializer.h"
#include <QDataStream>
#include <QIODevice>
#include <QDebug>
#include <QFile>
#include <memory>

namespace Serializer {

static void writeCommon(QDataStream &out, const DrawObject &obj)
{
    out << quint8(obj.type);
    out << obj.uuid;
    out << obj.color;
    out << qint32(obj.penWidth);
    out << obj.filled;
    out << obj.fillColor;
}

QByteArray serialize(const DrawObject &obj)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    writeCommon(out, obj);

    switch (obj.type) {
    case ObjectType::Stroke: {
        const auto &s = static_cast<const StrokeObject&>(obj);
        out << s.path;
        break;
    }
    case ObjectType::Rect: {
        const auto &r = static_cast<const RectObject&>(obj);
        out << r.rect;
        break;
    }
    case ObjectType::Ellipse: {
        const auto &e = static_cast<const EllipseObject&>(obj);
        out << e.rect;
        break;
    }
    case ObjectType::Triangle: {
        const auto &t = static_cast<const TriangleObject&>(obj);
        out << t.p1 << t.p2 << t.p3;
        break;
    }
    }
    return data;
}

std::shared_ptr<DrawObject> deserialize(const QByteArray &data)
{
    QDataStream in(data);
    in.setVersion(QDataStream::Qt_6_0);

    quint8  typeRaw  = 0;
    QUuid   uuid;
    QColor  color;
    qint32  penWidth = 1;
    bool    filled   = false;
    QColor  fillColor;

    in >> typeRaw >> uuid >> color >> penWidth >> filled >> fillColor;
    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }

    const ObjectType type = ObjectType(typeRaw);
    std::shared_ptr<DrawObject> result;

    switch (type) {
    case ObjectType::Stroke: {
        auto s = std::make_shared<StrokeObject>();
        in >> s->path;
        result = s;
        break;
    }
    case ObjectType::Rect: {
        auto r = std::make_shared<RectObject>();
        in >> r->rect;
        result = r;
        break;
    }
    case ObjectType::Ellipse: {
        auto e = std::make_shared<EllipseObject>();
        in >> e->rect;
        result = e;
        break;
    }
    case ObjectType::Triangle: {
        auto t = std::make_shared<TriangleObject>();
        in >> t->p1 >> t->p2 >> t->p3;
        result = t;
        break;
    }
    default:
        return nullptr;
    }

    if (in.status() != QDataStream::Ok) {
        return nullptr;
    }
    result->uuid      = uuid;
    result->color     = color;
    result->penWidth  = penWidth;
    result->filled    = filled;
    result->fillColor = fillColor;
    result->type      = type;
    return result;
}

void saveToFile(const QString &path,
                                       const QVector<std::shared_ptr<DrawObject>> &objects)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return;

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);
    out << kMagic;
    out << quint32(objects.size());
    for (const auto &obj : objects) {
        out << serialize(*obj);
    }
}
QVector<std::shared_ptr<DrawObject> > loadFromFile(const QString &path)
{
    QVector<std::shared_ptr<DrawObject> > result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return result;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    quint32 magic;
    in >> magic;
    if (magic != kMagic)
        return result;

    quint32 count;
    QByteArray data;

    in >> count;
    for (quint32 i = 0; i < count; ++i) {
        in >> data;
        auto obj = deserialize(data);
        if (obj) result.append(obj);
    }
    return result;
}
}
