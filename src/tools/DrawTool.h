#pragma once

#include "../data/DrawObject.h"

#include <QObject>
#include <QColor>
#include <QPointF>
#include <QGraphicsScene>

// базовый класс для всех инструментов.
// каждый инструмент знает текущий цвет/толщину и умеет
// реагировать на нажатие/движение/отпускание мыши.
class DrawTool : public QObject
{
    Q_OBJECT

public:
    explicit DrawTool(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~DrawTool() = default;

    // вызывается когда пользователь нажал кнопку мыши
    virtual void onMousePress(QPointF pos, QGraphicsScene *scene) = 0;
    // вызывается при движении мыши с зажатой кнопкой
    virtual void onMouseMove(QPointF pos, QGraphicsScene *scene) = 0;
    // вызывается когда отпустил кнопку — инструмент должен вернуть готовый объект
    virtual std::shared_ptr<DrawObject> onMouseRelease(QPointF pos, QGraphicsScene *scene) = 0;

    // текущие настройки инструмента
    void setColor(QColor c)    { m_color = c; }
    void setPenWidth(int w)    { m_penWidth = w; }
    void setFilled(bool f)     { m_filled = f; }
    void setFillColor(QColor c){ m_fillColor = c; }

    QColor color()     const { return m_color; }
    int    penWidth()  const { return m_penWidth; }

protected:
    QColor m_color     = Qt::black;
    int    m_penWidth  = 2;
    bool   m_filled    = false;
    QColor m_fillColor = Qt::white;

    // заполняем базовые поля объекта
    void applyStyle(DrawObject &obj) const {
        obj.color     = m_color;
        obj.penWidth  = m_penWidth;
        obj.filled    = m_filled;
        obj.fillColor = m_fillColor;
    }
};
