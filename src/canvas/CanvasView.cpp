#include "CanvasView.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QScrollBar>

class StrokeItem : public QGraphicsPathItem
{
public:
    using QGraphicsPathItem::QGraphicsPathItem;

    QPainterPath shape() const override
    {
        QPainterPath p;
        p.addRect(path().boundingRect());
        return p;
    }
};

CanvasView::CanvasView(BoardModel *model, QWidget *parent)
    : QGraphicsView(parent), m_model(model)
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-5000, -5000, 10000, 10000);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setBackgroundBrush(QBrush(Qt::white));
    setDragMode(QGraphicsView::NoDrag);

    m_pencilTool   = new PencilTool(this);
    m_rectTool     = new ShapeTool(ShapeTool::Shape::Rect, this);
    m_ellipseTool  = new ShapeTool(ShapeTool::Shape::Ellipse, this);
    m_triangleTool = new ShapeTool(ShapeTool::Shape::Triangle, this);
    m_eraserTool   = new EraserTool(this);
    m_fillTool     = new FillTool(this);

    connect(m_eraserTool, &EraserTool::eraseRequested,
            this, [this](QUuid uuid) { m_model->removeObject(uuid); });

    connect(m_fillTool, &FillTool::fillRequested,
            this, [this](QUuid uuid, QColor color) { m_model->fillObject(uuid, color); });

    connect(m_model, &BoardModel::objectAdded,   this, &CanvasView::onObjectAdded);
    connect(m_model, &BoardModel::objectRemoved, this, &CanvasView::onObjectRemoved);
    connect(m_model, &BoardModel::objectFilled,  this, &CanvasView::onObjectFilled);
    connect(m_model, &BoardModel::objectMoved,   this, &CanvasView::onObjectMoved);
    connect(m_model, &BoardModel::boardCleared,  this, &CanvasView::onBoardCleared);

    setToolPencil();
}

void CanvasView::commitSelectPositions()
{
    QList<std::pair<QUuid, QPointF>> moves;
    for (auto it = m_itemMap.begin(); it != m_itemMap.end(); ++it) {
        QPointF delta = it.value()->pos();
        if (!delta.isNull())
            moves.append({it.key(), delta});
    }
    for (const auto &m : moves)
        m_model->updateObjectPosition(m.first, m.second);
}

void CanvasView::syncItemGeometry(QGraphicsItem *item, const std::shared_ptr<DrawObject> &obj)
{
    switch (obj->type) {
    case ObjectType::Stroke:
        static_cast<QGraphicsPathItem*>(item)
            ->setPath(static_cast<StrokeObject*>(obj.get())->path);
        break;
    case ObjectType::Rect:
        static_cast<QGraphicsRectItem*>(item)
            ->setRect(static_cast<RectObject*>(obj.get())->rect);
        break;
    case ObjectType::Ellipse:
        static_cast<QGraphicsEllipseItem*>(item)
            ->setRect(static_cast<EllipseObject*>(obj.get())->rect);
        break;
    case ObjectType::Triangle: {
        auto *t = static_cast<TriangleObject*>(obj.get());
        QPolygonF poly; poly << t->p1 << t->p2 << t->p3;
        static_cast<QGraphicsPolygonItem*>(item)->setPolygon(poly);
        break;
    }
    }
}

void CanvasView::setItemsMovable(bool movable)
{
    for (QGraphicsItem *item : m_itemMap.values()) {
        item->setFlag(QGraphicsItem::ItemIsSelectable, movable);
        item->setFlag(QGraphicsItem::ItemIsMovable,    movable);
        if (!movable) item->setSelected(false);
    }
}

void CanvasView::setToolPencil()
{
    if (m_selectMode) { commitSelectPositions(); setItemsMovable(false); m_selectMode = false; }
    m_tool = m_pencilTool;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::CrossCursor);
    emit toolChanged("Карандаш");
}

void CanvasView::setToolRect()
{
    if (m_selectMode) { commitSelectPositions(); setItemsMovable(false); m_selectMode = false; }
    m_tool = m_rectTool;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::CrossCursor);
    emit toolChanged("Прямоугольник");
}

void CanvasView::setToolEllipse()
{
    if (m_selectMode) { commitSelectPositions(); setItemsMovable(false); m_selectMode = false; }
    m_tool = m_ellipseTool;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::CrossCursor);
    emit toolChanged("Эллипс");
}

void CanvasView::setToolTriangle()
{
    if (m_selectMode) { commitSelectPositions(); setItemsMovable(false); m_selectMode = false; }
    m_tool = m_triangleTool;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::CrossCursor);
    emit toolChanged("Треугольник");
}

void CanvasView::setToolEraser()
{
    if (m_selectMode) { commitSelectPositions(); setItemsMovable(false); m_selectMode = false; }
    m_tool = m_eraserTool;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::PointingHandCursor);
    emit toolChanged("Ластик");
}

void CanvasView::setToolFill()
{
    if (m_selectMode) { commitSelectPositions(); setItemsMovable(false); m_selectMode = false; }
    m_tool = m_fillTool;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::PointingHandCursor);
    emit toolChanged("Заливка");
}

void CanvasView::setToolSelect()
{
    m_selectMode = true;
    m_tool = nullptr;
    setDragMode(QGraphicsView::RubberBandDrag);
    setCursor(Qt::ArrowCursor);
    setItemsMovable(true);
    emit toolChanged("Выделение");
}

void CanvasView::setColor(const QColor &color)
{
    m_color = color;
    for (auto *t : {static_cast<DrawTool*>(m_pencilTool),
                    static_cast<DrawTool*>(m_rectTool),
                    static_cast<DrawTool*>(m_ellipseTool),
                    static_cast<DrawTool*>(m_triangleTool),
                    static_cast<DrawTool*>(m_fillTool)})
        t->setColor(color);
}

void CanvasView::setPenWidth(int width)
{
    m_penWidth = width;
    for (auto *t : {static_cast<DrawTool*>(m_pencilTool),
                    static_cast<DrawTool*>(m_rectTool),
                    static_cast<DrawTool*>(m_ellipseTool),
                    static_cast<DrawTool*>(m_triangleTool)})
        t->setPenWidth(width);
}

void CanvasView::setFilled(bool filled)
{
    m_filled = filled;
    for (auto *t : {m_rectTool, m_ellipseTool, m_triangleTool})
        t->setFilled(filled);
}

void CanvasView::applyZoom(double factor)
{
    double nz = m_zoomFactor * factor;
    if (nz < 0.1 || nz > 4.0) return;
    m_zoomFactor = nz;
    scale(factor, factor);
    emit zoomChanged(currentZoomPercent());
}

int  CanvasView::currentZoomPercent() const { return qRound(m_zoomFactor * 100.0); }
void CanvasView::zoomIn()  { applyZoom(1.25); }
void CanvasView::zoomOut() { applyZoom(0.8); }

void CanvasView::fitToWindow()
{
    fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    m_zoomFactor = transform().m11();
    emit zoomChanged(currentZoomPercent());
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setDragMode(QGraphicsView::ScrollHandDrag);
        QMouseEvent fake(event->type(), event->position(), event->globalPosition(),
                         Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
        return;
    }
    if (!m_selectMode && event->button() == Qt::LeftButton && m_tool) {
        m_drawing = true;
        m_tool->onMousePress(mapToScene(event->pos()), m_scene);
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_selectMode && m_drawing && m_tool)
        m_tool->onMouseMove(mapToScene(event->pos()), m_scene);
    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setDragMode(m_selectMode ? QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag);
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    if (!m_selectMode && event->button() == Qt::LeftButton && m_drawing && m_tool) {
        m_drawing = false;
        auto obj = m_tool->onMouseRelease(mapToScene(event->pos()), m_scene);
        if (obj) { m_model->addObject(obj); emit objectCreated(obj); }
    }
    QGraphicsView::mouseReleaseEvent(event);
    if (m_selectMode && event->button() == Qt::LeftButton)
        commitSelectPositions();
}

void CanvasView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        applyZoom(event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15);
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void CanvasView::onObjectAdded(std::shared_ptr<DrawObject> obj)
{
    if (m_itemMap.contains(obj->uuid)) return;
    QGraphicsItem *item = addItemForObject(obj);
    if (!item) return;
    item->setData(0, QVariant::fromValue(obj->uuid));
    item->setFlag(QGraphicsItem::ItemIsSelectable, m_selectMode);
    item->setFlag(QGraphicsItem::ItemIsMovable,    m_selectMode);
    m_itemMap[obj->uuid] = item;
}

void CanvasView::onObjectRemoved(QUuid uuid)
{
    auto it = m_itemMap.find(uuid);
    if (it == m_itemMap.end()) return;
    m_scene->removeItem(it.value());
    delete it.value();
    m_itemMap.erase(it);
}

void CanvasView::onObjectFilled(QUuid uuid, QColor color)
{
    auto it = m_itemMap.find(uuid);
    if (it == m_itemMap.end()) return;
    QBrush brush(color);
    if (auto *r = dynamic_cast<QGraphicsRectItem*>(it.value()))
        r->setBrush(brush);
    else if (auto *e = dynamic_cast<QGraphicsEllipseItem*>(it.value()))
        e->setBrush(brush);
    else if (auto *p = dynamic_cast<QGraphicsPolygonItem*>(it.value()))
        p->setBrush(brush);
}

void CanvasView::onObjectMoved(QUuid uuid, QPointF )
{
    auto it = m_itemMap.find(uuid);
    if (it == m_itemMap.end()) return;
    auto obj = m_model->findByUuid(uuid);
    if (!obj) return;
    syncItemGeometry(it.value(), obj);
    it.value()->setPos(0, 0);
}

void CanvasView::onBoardCleared()
{
    m_scene->clear();
    m_itemMap.clear();
}

QGraphicsItem* CanvasView::addItemForObject(std::shared_ptr<DrawObject> obj)
{
    QPen   pen(obj->color, obj->penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush brush = obj->filled ? QBrush(obj->fillColor) : QBrush(Qt::NoBrush);

    switch (obj->type) {
    case ObjectType::Stroke: {
        auto *s = static_cast<StrokeObject*>(obj.get());
        auto *item = new StrokeItem(s->path);
        item->setPen(pen);
        m_scene->addItem(item);
        return item;
    }
    case ObjectType::Rect: {
        auto *r = static_cast<RectObject*>(obj.get());
        return m_scene->addRect(r->rect, pen, brush);
    }
    case ObjectType::Ellipse: {
        auto *e = static_cast<EllipseObject*>(obj.get());
        return m_scene->addEllipse(e->rect, pen, brush);
    }
    case ObjectType::Triangle: {
        auto *t = static_cast<TriangleObject*>(obj.get());
        QPolygonF poly; poly << t->p1 << t->p2 << t->p3;
        return m_scene->addPolygon(poly, pen, brush);
    }
    }
    return nullptr;
}

void CanvasView::flushSelectMode()
{
    if (m_selectMode)
        commitSelectPositions();
}
