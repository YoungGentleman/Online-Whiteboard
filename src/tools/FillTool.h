#pragma once

#include "DrawTool.h"
#include <QGraphicsItem>

// инструмент "заливка ведром".
// при клике на фигуру (rect, ellipse, triangle) заливает её цветом.
// кривые (stroke) не заливаются — они не замкнуты.
//
// реализация через hit-test: scene->itemAt(pos) находит объект,
// затем мы меняем его цвет заливки через uuid в модели.
class FillTool : public DrawTool
{
    Q_OBJECT

signals:
    // canvas слушает этот сигнал и обновляет объект в модели
    void fillRequested(QUuid uuid, QColor color);

public:
    explicit FillTool(QObject *parent = nullptr) : DrawTool(parent) {}

    void onMousePress(QPointF pos, QGraphicsScene *scene) override
    {
        QGraphicsItem *item = scene->itemAt(pos, QTransform());
        if (!item) {
            qDebug() << "[FillTool] клик в пустоту";
            return;
        }

        QVariant uuidVar = item->data(0);
        if (!uuidVar.isValid()) {
            qDebug() << "[FillTool] item без uuid, пропускаем";
            return;
        }

        QUuid uuid = uuidVar.value<QUuid>();
        qDebug() << "[FillTool] заливаем объект uuid:"
                 << uuid.toString(QUuid::WithoutBraces).left(8)
                 << "цвет:" << m_color.name();

        emit fillRequested(uuid, m_color);
    }

    void onMouseMove(QPointF pos, QGraphicsScene *scene) override
    {
        Q_UNUSED(pos) Q_UNUSED(scene)
    }

    std::shared_ptr<DrawObject> onMouseRelease(QPointF pos, QGraphicsScene *scene) override
    {
        Q_UNUSED(pos) Q_UNUSED(scene)
        return nullptr; // заливка не создаёт новый объект
    }
};
