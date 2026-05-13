#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <memory>

#include "../data/BoardModel.h"
#include "../tools/DrawTool.h"
#include "../tools/PencilTool.h"
#include "../tools/ShapeTool.h"
#include "../tools/EraserTool.h"

// CanvasView — главный виджет рисования.
// Обёртывает QGraphicsView + QGraphicsScene и связывает
// пользовательский ввод с инструментами и моделью данных.
class CanvasView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CanvasView(BoardModel *model, QWidget *parent = nullptr);

    // переключение инструментов — вызывается из MainWindow
    void setToolPencil();
    void setToolRect();
    void setToolEllipse();
    void setToolTriangle();
    void setToolEraser();

    // настройки текущего инструмента
    void setColor(const QColor &color);
    void setPenWidth(int width);
    void setFilled(bool filled);

    // масштаб
    void zoomIn();
    void zoomOut();
    void fitToWindow();

    int currentZoomPercent() const;

signals:
    void zoomChanged(int percent);
    void toolChanged(const QString &name);
    // новый объект нарисован — нужно добавить в модель и отправить по сети
    void objectCreated(std::shared_ptr<DrawObject> obj);

public slots:
    // вызывается когда объект добавлен в модель (в т.ч. пришедший по сети)
    void onObjectAdded(std::shared_ptr<DrawObject> obj);
    void onObjectRemoved(QUuid uuid);
    void onBoardCleared();

protected:
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseMoveEvent(QMouseEvent *event)    override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event)        override;

private:
    QGraphicsItem* addItemForObject(std::shared_ptr<DrawObject> obj);
    void applyZoom(double factor);

    QGraphicsScene *m_scene   = nullptr;
    BoardModel     *m_model   = nullptr;
    DrawTool       *m_tool    = nullptr;
    bool            m_drawing = false;

    PencilTool   *m_pencilTool   = nullptr;
    ShapeTool    *m_rectTool     = nullptr;
    ShapeTool    *m_ellipseTool  = nullptr;
    ShapeTool    *m_triangleTool = nullptr;
    EraserTool   *m_eraserTool   = nullptr;

    double m_zoomFactor = 1.0;

    // uuid → item, чтобы быстро найти item для удаления
    QHash<QUuid, QGraphicsItem*> m_itemMap;

    QColor m_color    = Qt::black;
    int    m_penWidth = 2;
    bool   m_filled   = false;
};
