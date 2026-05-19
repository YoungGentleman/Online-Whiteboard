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

    void onMousePress(QPointF pos, QGraphicsScene *scene) override { tryErase(pos, scene); }
    void onMouseMove(QPointF pos, QGraphicsScene *scene)  override { tryErase(pos, scene); }

    std::shared_ptr<DrawObject> onMouseRelease(QPointF, QGraphicsScene*) override
    { return nullptr; }

private:
    void tryErase(QPointF pos, QGraphicsScene *scene)
    {
        QGraphicsItem *item = scene->itemAt(pos, QTransform());
        if (!item) return;
        QVariant v = item->data(0);
        if (v.isValid())
            emit eraseRequested(v.value<QUuid>());
    }
};
