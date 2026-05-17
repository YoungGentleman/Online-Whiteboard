#include "BoardModel.h"

#include <QDebug>

// вспомогательная функция — название типа объекта для лога
static const char* objectTypeName(ObjectType t)
{
    switch (t) {
    case ObjectType::Stroke:   return "Stroke";
    case ObjectType::Rect:     return "Rect";
    case ObjectType::Ellipse:  return "Ellipse";
    case ObjectType::Triangle: return "Triangle";
    }
    return "Unknown";
}

BoardModel::BoardModel(QObject *parent)
    : QObject(parent)
{
    qDebug() << "[BoardModel] создана";
}

void BoardModel::addObject(std::shared_ptr<DrawObject> obj)
{
    m_objects.append(obj);
    qDebug() << "[BoardModel] добавлен объект:"
             << objectTypeName(obj->type)
             << "uuid:" << obj->uuid.toString(QUuid::WithoutBraces).left(8)
             << "цвет:" << obj->color.name()
             << "толщина:" << obj->penWidth
             << "| всего объектов:" << m_objects.size();
    emit objectAdded(obj);
}

void BoardModel::removeObject(const QUuid &uuid)
{
    for (int i = 0; i < m_objects.size(); ++i) {
        if (m_objects[i]->uuid == uuid) {
            qDebug() << "[BoardModel] удалён объект:"
                     << objectTypeName(m_objects[i]->type)
                     << "uuid:" << uuid.toString(QUuid::WithoutBraces).left(8)
                     << "| осталось объектов:" << m_objects.size() - 1;
            m_objects.removeAt(i);
            emit objectRemoved(uuid);
            return;
        }
    }
    qDebug() << "[BoardModel] удаление: объект не найден, uuid:"
             << uuid.toString(QUuid::WithoutBraces).left(8);
}

void BoardModel::clear()
{
    qDebug() << "[BoardModel] очищена доска, было объектов:" << m_objects.size();
    m_objects.clear();
    emit boardCleared();
}

void BoardModel::fillObject(const QUuid &uuid, const QColor &color)
{
    auto obj = findByUuid(uuid);
    if (!obj) {
        qDebug() << "[BoardModel] fillObject: объект не найден";
        return;
    }
    if (obj->type == ObjectType::Stroke) {
        qDebug() << "[BoardModel] fillObject: кривые не заливаются";
        return;
    }
    obj->filled    = true;
    obj->fillColor = color;
    qDebug() << "[BoardModel] залит объект uuid:"
             << uuid.toString(QUuid::WithoutBraces).left(8)
             << "цвет:" << color.name();
    emit objectFilled(uuid, color);
}

std::shared_ptr<DrawObject> BoardModel::findByUuid(const QUuid &uuid) const
{
    for (auto &obj : m_objects) {
        if (obj->uuid == uuid)
            return obj;
    }
    return nullptr;
}
