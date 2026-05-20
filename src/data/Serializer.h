#pragma once

#include "DrawObject.h"

#include <QDataStream>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <memory>

static const quint32 WBD_MAGIC   = 0x57424421;
static const quint8  WBD_VERSION = 1;

class Serializer
{
public:
    static void writeObject(QDataStream &out, const DrawObject &obj)
    {
        out << static_cast<quint8>(obj.type);
        out << obj.uuid;
        out << obj.color;
        out << qint32(obj.penWidth);
        out << obj.filled;
        out << obj.fillColor;

        switch (obj.type) {
        case ObjectType::Stroke:
            out << static_cast<const StrokeObject&>(obj).path;
            break;
        case ObjectType::Rect:
            out << static_cast<const RectObject&>(obj).rect;
            break;
        case ObjectType::Ellipse:
            out << static_cast<const EllipseObject&>(obj).rect;
            break;
        case ObjectType::Triangle: {
            const auto &t = static_cast<const TriangleObject&>(obj);
            out << t.p1 << t.p2 << t.p3;
            break;
        }
        }
    }

    static std::shared_ptr<DrawObject> readObject(QDataStream &in)
    {
        quint8 typeRaw;
        in >> typeRaw;
        if (in.status() != QDataStream::Ok) return nullptr;
        ObjectType type = static_cast<ObjectType>(typeRaw);

        QUuid   uuid;
        QColor  color;
        qint32  penWidth;
        bool    filled;
        QColor  fillColor;
        in >> uuid >> color >> penWidth >> filled >> fillColor;
        if (in.status() != QDataStream::Ok) return nullptr;

        std::shared_ptr<DrawObject> obj;
        switch (type) {
        case ObjectType::Stroke: {
            auto s = std::make_shared<StrokeObject>();
            in >> s->path;
            obj = s;
            break;
        }
        case ObjectType::Rect: {
            auto r = std::make_shared<RectObject>();
            in >> r->rect;
            obj = r;
            break;
        }
        case ObjectType::Ellipse: {
            auto e = std::make_shared<EllipseObject>();
            in >> e->rect;
            obj = e;
            break;
        }
        case ObjectType::Triangle: {
            auto t = std::make_shared<TriangleObject>();
            in >> t->p1 >> t->p2 >> t->p3;
            obj = t;
            break;
        }
        default:
            return nullptr;
        }

        if (in.status() != QDataStream::Ok) return nullptr;

        obj->uuid      = uuid;
        obj->color     = color;
        obj->penWidth  = int(penWidth);
        obj->filled    = filled;
        obj->fillColor = fillColor;
        obj->type      = type;
        return obj;
    }

    static QByteArray serializeObject(const DrawObject &obj)
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        writeObject(out, obj);
        return data;
    }

    static std::shared_ptr<DrawObject> deserializeObject(const QByteArray &data)
    {
        QDataStream in(data);
        in.setVersion(QDataStream::Qt_6_0);
        return readObject(in);
    }

    static bool saveToFile(const QString &path,
                           const QVector<std::shared_ptr<DrawObject>> &objects)
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) return false;
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);
        out << WBD_MAGIC << WBD_VERSION;
        out << quint32(objects.size());
        for (const auto &obj : objects)
            writeObject(out, *obj);
        return true;
    }

    static QVector<std::shared_ptr<DrawObject>> loadFromFile(const QString &path)
    {
        QVector<std::shared_ptr<DrawObject>> result;
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return result;
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);
        quint32 magic; quint8 version;
        in >> magic >> version;
        if (magic != WBD_MAGIC || version != WBD_VERSION) return result;
        quint32 count;
        in >> count;
        for (quint32 i = 0; i < count; ++i) {
            auto obj = readObject(in);
            if (obj) result.append(obj);
        }
        return result;
    }
};
