#pragma once
#include <QUuid>
#include <QColor>
#include <QPainterPath>
#include <QRectF>
#include <QDataStream>

enum class ObjectType : quint8 {
    Stroke   = 1,
    Rect     = 2,
    Ellipse  = 3,
    Triangle = 4,
};

struct DrawObject {
    QUuid      uuid      = QUuid::createUuid();
    ObjectType type;
    QColor     color     = Qt::black;
    int        penWidth  = 2;
    bool       filled    = false;
    QColor     fillColor = Qt::white;
};

struct StrokeObject : DrawObject {
    QPainterPath path;
    StrokeObject() { type = ObjectType::Stroke; }
};

struct RectObject : DrawObject {
    QRectF rect;
    RectObject() { type = ObjectType::Rect; }
};

struct EllipseObject : DrawObject {
    QRectF rect;
    EllipseObject() { type = ObjectType::Ellipse; }
};

struct TriangleObject : DrawObject {
    QPointF p1, p2, p3;
    TriangleObject() { type = ObjectType::Triangle; }
};
