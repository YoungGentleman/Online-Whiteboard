#pragma once

#include "DrawTool.h"
#include <QGraphicsPathItem>

// инструмент "карандаш" — рисует свободную кривую.
// вся кривая от нажатия до отпускания = один StrokeObject.
class PencilTool : public DrawTool
{
    Q_OBJECT

public:
    explicit PencilTool(QObject *parent = nullptr) : DrawTool(parent) {}

    void onMousePress(QPointF pos, QGraphicsScene *scene) override
    {
        // начинаем новый путь
        m_path = QPainterPath();
        m_path.moveTo(pos);

        // сразу добавляем временный item на сцену, чтобы видеть линию в реальном времени
        QPen pen(m_color, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        m_tempItem = scene->addPath(m_path, pen);
    }

    void onMouseMove(QPointF pos, QGraphicsScene *scene) override
    {
        Q_UNUSED(scene)
        if (!m_tempItem) return;

        m_path.lineTo(pos);
        m_tempItem->setPath(m_path);
    }

    std::shared_ptr<DrawObject> onMouseRelease(QPointF pos, QGraphicsScene *scene) override
    {
        Q_UNUSED(scene)
        if (!m_tempItem) return nullptr;

        m_path.lineTo(pos);

        // убираем временный item — canvas добавит постоянный через BoardModel
        scene->removeItem(m_tempItem);
        delete m_tempItem;
        m_tempItem = nullptr;

        auto stroke = std::make_shared<StrokeObject>();
        applyStyle(*stroke);
        stroke->path = m_path;
        return stroke;
    }

private:
    QPainterPath      m_path;
    QGraphicsPathItem *m_tempItem = nullptr;
};
