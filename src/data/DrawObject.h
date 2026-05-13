#pragma once

#include <QUuid>
#include <QColor>
#include <QPainterPath>
#include <QRectF>
#include <QDataStream>

// тип объекта на доске
enum class ObjectType : quint8 {
    Stroke   = 1,  // кривая карандашом
    Rect     = 2,  // прямоугольник
    Ellipse  = 3,  // эллипс
    Triangle = 4,  // треугольник
};

// базовый объект — есть у каждой фигуры
struct DrawObject {
    QUuid      uuid = QUuid::createUuid();
    ObjectType type;
    QColor     color    = Qt::black;
    int        penWidth = 2;
    bool       filled   = false;
    QColor     fillColor = Qt::white;
};

// кривая карандашом
struct StrokeObject : DrawObject {
    QPainterPath path;
    StrokeObject() { type = ObjectType::Stroke; }
};

// прямоугольник
struct RectObject : DrawObject {
    QRectF rect;
    RectObject() { type = ObjectType::Rect; }
};

// эллипс
struct EllipseObject : DrawObject {
    QRectF rect;
    EllipseObject() { type = ObjectType::Ellipse; }
};

// треугольник — три точки
struct TriangleObject : DrawObject {
    QPointF p1, p2, p3;
    TriangleObject() { type = ObjectType::Triangle; }
};
