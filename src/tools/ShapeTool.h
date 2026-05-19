#pragma once

#include "DrawTool.h"
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>

class ShapeTool : public DrawTool
{
    Q_OBJECT

public:
    enum class Shape { Rect, Ellipse, Triangle };

    explicit ShapeTool(Shape shape, QObject *parent = nullptr)
        : DrawTool(parent), m_shape(shape) {}

    void onMousePress(QPointF pos, QGraphicsScene *scene) override
    {
        m_startPos = pos;
        m_tempItem = nullptr;
        QPen   pen(m_color, m_penWidth);
        QBrush brush = m_filled ? QBrush(m_fillColor) : QBrush(Qt::NoBrush);
        switch (m_shape) {
        case Shape::Rect:
            m_tempItem = scene->addRect(QRectF(pos, pos), pen, brush);     break;
        case Shape::Ellipse:
            m_tempItem = scene->addEllipse(QRectF(pos, pos), pen, brush);  break;
        case Shape::Triangle:
            m_tempItem = scene->addPolygon(QPolygonF(), pen, brush);       break;
        }
    }

    void onMouseMove(QPointF pos, QGraphicsScene *) override
    {
        if (!m_tempItem) return;
        QRectF rect = normalizedRect(m_startPos, pos);
        switch (m_shape) {
        case Shape::Rect:
            static_cast<QGraphicsRectItem*>(m_tempItem)->setRect(rect);          break;
        case Shape::Ellipse:
            static_cast<QGraphicsEllipseItem*>(m_tempItem)->setRect(rect);       break;
        case Shape::Triangle:
            static_cast<QGraphicsPolygonItem*>(m_tempItem)->setPolygon(makeTriangle(rect)); break;
        }
    }

    std::shared_ptr<DrawObject> onMouseRelease(QPointF pos, QGraphicsScene *scene) override
    {
        if (!m_tempItem) return nullptr;
        scene->removeItem(m_tempItem);
        delete m_tempItem;
        m_tempItem = nullptr;

        QRectF rect = normalizedRect(m_startPos, pos);
        if (rect.width() < 3 && rect.height() < 3) return nullptr;

        switch (m_shape) {
        case Shape::Rect: {
            auto obj = std::make_shared<RectObject>();
            applyStyle(*obj); obj->rect = rect; return obj;
        }
        case Shape::Ellipse: {
            auto obj = std::make_shared<EllipseObject>();
            applyStyle(*obj); obj->rect = rect; return obj;
        }
        case Shape::Triangle: {
            auto poly = makeTriangle(rect);
            auto obj  = std::make_shared<TriangleObject>();
            applyStyle(*obj);
            obj->p1 = poly[0]; obj->p2 = poly[1]; obj->p3 = poly[2];
            return obj;
        }
        }
        return nullptr;
    }

private:
    Shape          m_shape;
    QPointF        m_startPos;
    QGraphicsItem *m_tempItem = nullptr;

    static QRectF normalizedRect(QPointF a, QPointF b) {
        return QRectF(QPointF(qMin(a.x(), b.x()), qMin(a.y(), b.y())),
                      QPointF(qMax(a.x(), b.x()), qMax(a.y(), b.y())));
    }

    static QPolygonF makeTriangle(const QRectF &r) {
        QPolygonF p;
        p << QPointF(r.left() + r.width() / 2, r.top())
          << QPointF(r.left(),  r.bottom())
          << QPointF(r.right(), r.bottom());
        return p;
    }
};
