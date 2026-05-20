#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <memory>

#include "../data/BoardModel.h"
#include "../tools/DrawTool.h"
#include "../tools/PencilTool.h"
#include "../tools/ShapeTool.h"
#include "../tools/EraserTool.h"
#include "../tools/FillTool.h"

class CanvasView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CanvasView(BoardModel *model, QWidget *parent = nullptr);

    void setToolPencil();
    void setToolRect();
    void setToolEllipse();
    void setToolTriangle();
    void setToolEraser();
    void setToolFill();
    void setToolSelect();

    void flushSelectMode();

    void setColor(const QColor &color);
    void setPenWidth(int width);
    void setFilled(bool filled);

    void zoomIn();
    void zoomOut();
    void fitToWindow();
    int  currentZoomPercent() const;

signals:
    void zoomChanged(int percent);
    void toolChanged(const QString &name);
    void objectCreated(std::shared_ptr<DrawObject> obj);

public slots:
    void onObjectAdded(std::shared_ptr<DrawObject> obj);
    void onObjectRemoved(QUuid uuid);
    void onObjectFilled(QUuid uuid, QColor color);
    void onObjectMoved(QUuid uuid, QPointF delta);
    void onBoardCleared();

protected:
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseMoveEvent(QMouseEvent *event)    override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event)        override;

private:
    QGraphicsItem* addItemForObject(std::shared_ptr<DrawObject> obj);
    void           syncItemGeometry(QGraphicsItem *item, const std::shared_ptr<DrawObject> &obj);
    void           applyZoom(double factor);
    void           commitSelectPositions();
    void           setItemsMovable(bool movable);

    QGraphicsScene *m_scene        = nullptr;
    BoardModel     *m_model        = nullptr;
    DrawTool       *m_tool         = nullptr;
    bool            m_drawing      = false;
    bool            m_selectMode   = false;

    PencilTool     *m_pencilTool   = nullptr;
    ShapeTool      *m_rectTool     = nullptr;
    ShapeTool      *m_ellipseTool  = nullptr;
    ShapeTool      *m_triangleTool = nullptr;
    EraserTool     *m_eraserTool   = nullptr;
    FillTool       *m_fillTool     = nullptr;

    double m_zoomFactor = 1.0;
    QHash<QUuid, QGraphicsItem*> m_itemMap;

    QColor m_color    = Qt::black;
    int    m_penWidth = 2;
    bool   m_filled   = false;
};

