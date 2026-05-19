#pragma once

#include "DrawTool.h"
#include <QGraphicsItem>

class FillTool : public DrawTool
{
    Q_OBJECT

signals:
    void fillRequested(QUuid uuid, QColor color);

public:
    explicit FillTool(QObject *parent = nullptr) : DrawTool(parent) {}

    void onMousePress(QPointF pos, QGraphicsScene *scene) override
    {
        QGraphicsItem *item = scene->itemAt(pos, QTransform());
        if (!item) return;
        QVariant v = item->data(0);
        if (!v.isValid()) return;
        emit fillRequested(v.value<QUuid>(), m_color);
    }

    void onMouseMove(QPointF, QGraphicsScene*) override {}

    std::shared_ptr<DrawObject> onMouseRelease(QPointF, QGraphicsScene*) override
    { return nullptr; }
};
