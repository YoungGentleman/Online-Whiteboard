#pragma once

#include "DrawTool.h"
#include <QGraphicsPathItem>

class PencilTool : public DrawTool
{
    Q_OBJECT

public:
    explicit PencilTool(QObject *parent = nullptr) : DrawTool(parent) {}

    void onMousePress(QPointF pos, QGraphicsScene *scene) override
    {
        m_path = QPainterPath();
        m_path.moveTo(pos);
        QPen pen(m_color, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        m_tempItem = scene->addPath(m_path, pen);
    }

    void onMouseMove(QPointF pos, QGraphicsScene *) override
    {
        if (!m_tempItem) return;
        m_path.lineTo(pos);
        m_tempItem->setPath(m_path);
    }

    std::shared_ptr<DrawObject> onMouseRelease(QPointF pos, QGraphicsScene *scene) override
    {
        if (!m_tempItem) return nullptr;
        m_path.lineTo(pos);
        scene->removeItem(m_tempItem);
        delete m_tempItem;
        m_tempItem = nullptr;

        auto stroke = std::make_shared<StrokeObject>();
        applyStyle(*stroke);
        stroke->path = m_path;
        return stroke;
    }

private:
    QPainterPath       m_path;
    QGraphicsPathItem *m_tempItem = nullptr;
};
