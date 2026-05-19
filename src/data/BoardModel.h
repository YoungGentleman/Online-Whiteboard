#pragma once
#include "DrawObject.h"
#include <QObject>
#include <QVector>
#include <memory>

// BoardModel хранит все объекты доски в порядке отрисовки.
// В будущем (коммит 3) он же будет сериализоваться и гоняться по сети.
class BoardModel : public QObject
{
    Q_OBJECT

public:
    explicit BoardModel(QObject *parent = nullptr);

    void addObject(std::shared_ptr<DrawObject> obj);
    void removeObject(const QUuid &uuid);
    void clear();

    // ищем объект по uuid — нужно для удаления по сети
    std::shared_ptr<DrawObject> findByUuid(const QUuid &uuid) const;

    const QVector<std::shared_ptr<DrawObject>>& objects() const { return m_objects; }

signals:
    void objectAdded(std::shared_ptr<DrawObject> obj);
    void objectRemoved(QUuid uuid);
    void boardCleared();

private:
    QVector<std::shared_ptr<DrawObject>> m_objects;
};
