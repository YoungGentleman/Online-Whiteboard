#include "BoardModel.h"

BoardModel::BoardModel(QObject *parent) : QObject(parent) {}

void BoardModel::addObject(std::shared_ptr<DrawObject> obj, bool fromNetwork)
{
    m_objects.append(obj);
    emit objectAdded(obj);
    if (!fromNetwork)
        emit localObjectCreated(obj);
}

void BoardModel::removeObject(const QUuid &uuid, bool fromNetwork)
{
    for (int i = 0; i < m_objects.size(); ++i) {
        if (m_objects[i]->uuid == uuid) {
            m_objects.removeAt(i);
            emit objectRemoved(uuid);
            if (!fromNetwork)
                emit localObjectErased(uuid);
            return;
        }
    }
}

void BoardModel::fillObject(const QUuid &uuid, const QColor &color, bool fromNetwork)
{
    auto obj = findByUuid(uuid);
    if (!obj || obj->type == ObjectType::Stroke)
        return;
    obj->filled    = true;
    obj->fillColor = color;
    emit objectFilled(uuid, color);
    if (!fromNetwork)
        emit localObjectFilled(uuid, color);
}

void BoardModel::updateObjectPosition(const QUuid &uuid, const QPointF &delta, bool fromNetwork)
{
    auto obj = findByUuid(uuid);
    if (!obj) return;

    switch (obj->type) {
    case ObjectType::Stroke:
        static_cast<StrokeObject*>(obj.get())->path.translate(delta);
        break;
    case ObjectType::Rect:
        static_cast<RectObject*>(obj.get())->rect.translate(delta);
        break;
    case ObjectType::Ellipse:
        static_cast<EllipseObject*>(obj.get())->rect.translate(delta);
        break;
    case ObjectType::Triangle: {
        auto *t = static_cast<TriangleObject*>(obj.get());
        t->p1 += delta;
        t->p2 += delta;
        t->p3 += delta;
        break;
    }
    }

    emit objectMoved(uuid, delta);
    if (!fromNetwork)
        emit localObjectMoved(uuid, delta);
}

void BoardModel::clear()
{
    m_objects.clear();
    emit boardCleared();
}

std::shared_ptr<DrawObject> BoardModel::findByUuid(const QUuid &uuid) const
{
    for (auto &obj : m_objects)
        if (obj->uuid == uuid)
            return obj;
    return nullptr;
}
