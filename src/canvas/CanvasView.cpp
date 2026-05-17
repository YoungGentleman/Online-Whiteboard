#include "CanvasView.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QScrollBar>
#include <QDebug>

CanvasView::CanvasView(BoardModel *model, QWidget *parent)
    : QGraphicsView(parent), m_model(model)
{
    // создаём сцену — бесконечный холст
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-5000, -5000, 10000, 10000);
    setScene(m_scene);

    // настройки вида
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setBackgroundBrush(QBrush(Qt::white));
    setDragMode(QGraphicsView::NoDrag);

    // создаём все инструменты
    m_pencilTool   = new PencilTool(this);
    m_rectTool     = new ShapeTool(ShapeTool::Shape::Rect, this);
    m_ellipseTool  = new ShapeTool(ShapeTool::Shape::Ellipse, this);
    m_triangleTool = new ShapeTool(ShapeTool::Shape::Triangle, this);
    m_eraserTool   = new EraserTool(this);
    m_fillTool     = new FillTool(this);

    // слушаем сигнал ластика
    connect(m_eraserTool, &EraserTool::eraseRequested,
            this, [this](QUuid uuid) {
                m_model->removeObject(uuid);
            });

    // слушаем сигнал заливки
    connect(m_fillTool, &FillTool::fillRequested,
            this, [this](QUuid uuid, QColor color) {
                m_model->fillObject(uuid, color);
            });

    // слушаем модель
    connect(m_model, &BoardModel::objectAdded,
            this, &CanvasView::onObjectAdded);
    connect(m_model, &BoardModel::objectRemoved,
            this, &CanvasView::onObjectRemoved);
    connect(m_model, &BoardModel::objectFilled,
            this, &CanvasView::onObjectFilled);
    connect(m_model, &BoardModel::boardCleared,
            this, &CanvasView::onBoardCleared);

    // по умолчанию — карандаш
    setToolPencil();
    qDebug() << "[CanvasView] инициализирован, сцена:" << m_scene->sceneRect();
}

// ---------------------------------------------------------------------------
// переключение инструментов
// ---------------------------------------------------------------------------

void CanvasView::setToolPencil()
{
    m_tool = m_pencilTool;
    setCursor(Qt::CrossCursor);
    qDebug() << "[CanvasView] инструмент: Карандаш";
    emit toolChanged("Карандаш");
}

void CanvasView::setToolRect()
{
    m_tool = m_rectTool;
    setCursor(Qt::CrossCursor);
    qDebug() << "[CanvasView] инструмент: Прямоугольник";
    emit toolChanged("Прямоугольник");
}

void CanvasView::setToolEllipse()
{
    m_tool = m_ellipseTool;
    setCursor(Qt::CrossCursor);
    qDebug() << "[CanvasView] инструмент: Эллипс";
    emit toolChanged("Эллипс");
}

void CanvasView::setToolTriangle()
{
    m_tool = m_triangleTool;
    setCursor(Qt::CrossCursor);
    qDebug() << "[CanvasView] инструмент: Треугольник";
    emit toolChanged("Треугольник");
}

void CanvasView::setToolEraser()
{
    m_tool = m_eraserTool;
    setCursor(Qt::PointingHandCursor);
    qDebug() << "[CanvasView] инструмент: Ластик";
    emit toolChanged("Ластик");
}

void CanvasView::setToolFill()
{
    m_tool = m_fillTool;
    // курсор в виде руки — напоминает ведро
    setCursor(Qt::PointingHandCursor);
    qDebug() << "[CanvasView] инструмент: Заливка";
    emit toolChanged("Заливка");
}

// ---------------------------------------------------------------------------
// настройки стиля
// ---------------------------------------------------------------------------

void CanvasView::setColor(const QColor &color)
{
    m_color = color;
    QList<DrawTool*> tools = {m_pencilTool, m_rectTool, m_ellipseTool,
                               m_triangleTool, m_fillTool};
    for (auto *t : tools)
        t->setColor(color);
}

void CanvasView::setPenWidth(int width)
{
    m_penWidth = width;
    QList<DrawTool*> tools = {m_pencilTool, m_rectTool, m_ellipseTool, m_triangleTool};
    for (auto *t : tools)
        t->setPenWidth(width);
}

void CanvasView::setFilled(bool filled)
{
    m_filled = filled;
    QList<DrawTool*> tools = {m_rectTool, m_ellipseTool, m_triangleTool};
    for (auto *t : tools)
        t->setFilled(filled);
}

// ---------------------------------------------------------------------------
// масштаб
// ---------------------------------------------------------------------------

void CanvasView::applyZoom(double factor)
{
    double newZoom = m_zoomFactor * factor;
    // ограничиваем 10%–400%
    if (newZoom < 0.1 || newZoom > 4.0) {
        qDebug() << "[CanvasView] зум: достигнут предел, текущий:" << currentZoomPercent() << "%";
        return;
    }
    m_zoomFactor = newZoom;
    scale(factor, factor);
    qDebug() << "[CanvasView] зум:" << currentZoomPercent() << "%";
    emit zoomChanged(currentZoomPercent());
}

int CanvasView::currentZoomPercent() const
{
    return qRound(m_zoomFactor * 100.0);
}

void CanvasView::zoomIn()
{
    applyZoom(1.25);
}

void CanvasView::zoomOut()
{
    applyZoom(0.8);
}

void CanvasView::fitToWindow()
{
    fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    // пересчитываем m_zoomFactor
    m_zoomFactor = transform().m11();
    emit zoomChanged(currentZoomPercent());
}

// ---------------------------------------------------------------------------
// события мыши
// ---------------------------------------------------------------------------

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        // средняя кнопка — панорамирование через ScrollHandDrag
        setDragMode(QGraphicsView::ScrollHandDrag);
        // эмулируем нажатие левой кнопки чтобы QGraphicsView начал тащить
        QMouseEvent fakeEvent(event->type(), event->position(),
                              event->globalPosition(),
                              Qt::LeftButton, Qt::LeftButton,
                              event->modifiers());
        QGraphicsView::mousePressEvent(&fakeEvent);
        return;
    }

    if (event->button() == Qt::LeftButton && m_tool) {
        m_drawing = true;
        QPointF scenePos = mapToScene(event->pos());
        m_tool->onMousePress(scenePos, m_scene);
    }

    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drawing && m_tool) {
        QPointF scenePos = mapToScene(event->pos());
        m_tool->onMouseMove(scenePos, m_scene);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setDragMode(QGraphicsView::NoDrag);
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton && m_drawing && m_tool) {
        m_drawing = false;
        QPointF scenePos = mapToScene(event->pos());
        auto obj = m_tool->onMouseRelease(scenePos, m_scene);

        if (obj) {
            qDebug() << "[CanvasView] объект нарисован, передаём в модель";
            m_model->addObject(obj);
            emit objectCreated(obj);
        } else {
            qDebug() << "[CanvasView] объект слишком маленький, игнорируем";
        }
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void CanvasView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Ctrl+колесо — масштаб
        double factor = (event->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
        applyZoom(factor);
        event->accept();
    } else {
        // просто прокрутка
        QGraphicsView::wheelEvent(event);
    }
}

// ---------------------------------------------------------------------------
// слоты модели — синхронизируем сцену с данными
// ---------------------------------------------------------------------------

void CanvasView::onObjectAdded(std::shared_ptr<DrawObject> obj)
{
    // объект уже на сцене — мы сами его только что нарисовали
    if (m_itemMap.contains(obj->uuid)) {
        qDebug() << "[CanvasView] onObjectAdded: объект уже на сцене, пропускаем";
        return;
    }

    qDebug() << "[CanvasView] onObjectAdded: рисуем объект из модели (например, пришёл по сети)";
    QGraphicsItem *item = addItemForObject(obj);
    if (item) {
        item->setData(0, QVariant::fromValue(obj->uuid));
        m_itemMap[obj->uuid] = item;
    }
}

void CanvasView::onObjectRemoved(QUuid uuid)
{
    auto it = m_itemMap.find(uuid);
    if (it == m_itemMap.end()) {
        qDebug() << "[CanvasView] onObjectRemoved: item не найден на сцене";
        return;
    }

    qDebug() << "[CanvasView] onObjectRemoved: удаляем item со сцены";
    m_scene->removeItem(it.value());
    delete it.value();
    m_itemMap.erase(it);
}

void CanvasView::onObjectFilled(QUuid uuid, QColor color)
{
    auto it = m_itemMap.find(uuid);
    if (it == m_itemMap.end()) {
        qDebug() << "[CanvasView] onObjectFilled: item не найден";
        return;
    }

    QBrush brush(color);

    // обновляем заливку у нужного типа item
    if (auto *r = dynamic_cast<QGraphicsRectItem*>(it.value())) {
        r->setBrush(brush);
    } else if (auto *e = dynamic_cast<QGraphicsEllipseItem*>(it.value())) {
        e->setBrush(brush);
    } else if (auto *p = dynamic_cast<QGraphicsPolygonItem*>(it.value())) {
        p->setBrush(brush);
    } else {
        qDebug() << "[CanvasView] onObjectFilled: неподдерживаемый тип item";
    }
}

void CanvasView::onBoardCleared()
{
    qDebug() << "[CanvasView] очистка сцены, item на сцене:" << m_itemMap.size();
    m_scene->clear();
    m_itemMap.clear();
}

// ---------------------------------------------------------------------------
// создание QGraphicsItem по типу объекта
// ---------------------------------------------------------------------------

QGraphicsItem* CanvasView::addItemForObject(std::shared_ptr<DrawObject> obj)
{
    QPen pen(obj->color, obj->penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush brush = obj->filled ? QBrush(obj->fillColor) : QBrush(Qt::NoBrush);

    switch (obj->type) {
    case ObjectType::Stroke: {
        auto *stroke = static_cast<StrokeObject*>(obj.get());
        return m_scene->addPath(stroke->path, pen);
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
        QPolygonF poly;
        poly << t->p1 << t->p2 << t->p3;
        return m_scene->addPolygon(poly, pen, brush);
    }
    }
    return nullptr;
}
