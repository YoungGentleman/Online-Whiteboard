#pragma once

#include "DrawTool.h"
#include <QGraphicsItem>

class EraserTool : public DrawTool
{
    Q_OBJECT

signals:
    void eraseRequested(QUuid uuid);

public:
    explicit EraserTool(QObject *parent = nullptr) : DrawTool(parent) {}

    void onMousePress(QPointF pos, QGraphicsScene *scene) override
    {
        tryErase(pos, scene);
    }

    void onMouseMove(QPointF pos, QGraphicsScene *scene) override
    {
        // при движении с зажатой кнопкой тоже стираем
        tryErase(pos, scene);
    }

    std::shared_ptr<DrawObject> onMouseRelease(QPointF pos, QGraphicsScene *scene) override
    {
        Q_UNUSED(pos) Q_UNUSED(scene)
        return nullptr; // ластик ничего не создаёт
    }

private:
    void tryErase(QPointF pos, QGraphicsScene *scene)
    {
        // ищем item под курсором
        QGraphicsItem *item = scene->itemAt(pos, QTransform());
        if (!item) return;

        // каждый наш item хранит uuid объекта в data(0)
        QVariant uuidVar = item->data(0);
        if (!uuidVar.isValid()) return;

        emit eraseRequested(uuidVar.value<QUuid>());
    }
};
