#pragma once

#include "DrawObject.h"

#include <QObject>
#include <QVector>
#include <memory>

class BoardModel : public QObject
{
    Q_OBJECT

public:
    explicit BoardModel(QObject *parent = nullptr);

    void addObject(std::shared_ptr<DrawObject> obj, bool fromNetwork = false);
    void removeObject(const QUuid &uuid, bool fromNetwork = false);
    void fillObject(const QUuid &uuid, const QColor &color, bool fromNetwork = false);
    void updateObjectPosition(const QUuid &uuid, const QPointF &delta, bool fromNetwork = false);
    void clear();

    std::shared_ptr<DrawObject> findByUuid(const QUuid &uuid) const;
    const QVector<std::shared_ptr<DrawObject>>& objects() const { return m_objects; }

signals:
    void objectAdded(std::shared_ptr<DrawObject> obj);
    void objectRemoved(QUuid uuid);
    void objectFilled(QUuid uuid, QColor color);
    void objectMoved(QUuid uuid, QPointF delta);
    void boardCleared();

    void localObjectCreated(std::shared_ptr<DrawObject> obj);
    void localObjectErased(QUuid uuid);
    void localObjectFilled(QUuid uuid, QColor color);
    void localObjectMoved(QUuid uuid, QPointF delta);

private:
    QVector<std::shared_ptr<DrawObject>> m_objects;
};
