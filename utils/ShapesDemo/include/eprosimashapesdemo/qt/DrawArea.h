/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef DRAWAREA_H
#define DRAWAREA_H

#include <QBrush>
#include <QPen>
#include <QWidget>
#include <QTimer>
#include "eprosimashapesdemo/shapesdemo/Shape.h"
class ShapesDemo;

#define SD_QT_COLOR_TRANS 255

const QColor SD_QT_PURPLE = QColor(125,38,205,SD_QT_COLOR_TRANS);
const QColor SD_QT_BLUE = QColor(0,0,255,SD_QT_COLOR_TRANS);
const QColor SD_QT_RED = QColor(255,0,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_GREEN = QColor(0,255,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_YELLOW = QColor(255,255,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_CYAN = QColor(0,255,255,SD_QT_COLOR_TRANS);
const QColor SD_QT_MAGENTA = QColor(255,20,147,SD_QT_COLOR_TRANS);
const QColor SD_QT_ORANGE = QColor(255,140,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_GRAY = QColor(190,190,190,SD_QT_COLOR_TRANS);

const QColor SD_QT_BLACK = QColor(0,0,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_WHITE = QColor(255,255,255,255);

inline QColor SD_COLOR2QColor(SD_COLOR& color)
{
    switch(color)
    {
    case SD_PURPLE: return SD_QT_PURPLE;
    case SD_BLUE: return SD_QT_BLUE;
    case SD_RED: return SD_QT_RED;
    case SD_GREEN: return SD_QT_GREEN;
    case SD_YELLOW: return SD_QT_YELLOW;
    case SD_CYAN: return SD_QT_CYAN;
    case SD_MAGENTA: return SD_QT_MAGENTA;
    case SD_ORANGE: return SD_QT_ORANGE;
    case SD_GRAY: return SD_QT_GRAY;
    case SD_BLACK: return SD_QT_BLACK;
    }
    return SD_QT_BLACK;
}





class QPainter;
class Shape;
class ShapeSubscriber;
/**
 * @brief Class DrawArea, implements the methods to draw the shapes in the draw widget area.
 */
class DrawArea: public QWidget
{
    Q_OBJECT
public:
    DrawArea(QWidget* parent=0);
    virtual ~DrawArea();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    /**
     * @brief setShapesDemo sets a pointer to the ShapesDemo class.
     * @param SD Pointer to the ShapesDemo object.
     */
    void setShapesDemo(ShapesDemo* SD);
    /**
     * @brief drawShapes Draw all shapes using the painter
     * @param painter Pointer to the painter object.
     */
    void drawShapes(QPainter* painter);
    /**
     * @brief stopTimer Stops the drawing timer.
     */
    void stopTimer(){this->killTimer(m_timerId);}

    void addContentFilter(ShapeSubscriber* ssub);

protected:
    /**
     * @brief paintEvent Paintevent method.
     * @param event Pointer to the event.
     */
    void paintEvent(QPaintEvent *event);
    /**
     * @brief timerEvent Timer event method.
     * @param event Pointer to the envent.
     */
    void timerEvent(QTimerEvent *event);

private:
    QPen m_pen;
    QBrush m_brush;
    Shape m_shape;
    /**
     * @brief paintShape method to pain a specific shape.
     * @param painter Pointer to the painter.
     * @param type TYpe of shape to paint.
     * @param sh Reference to the shape colors and dimensions.
     * @param alpha Transparency level.
     * @param isHistory Whether is part of history or not.
     */
    void paintShape(QPainter*painter, Shape& sh, uint8_t alpha=255,bool isHistory=false);

    ShapesDemo* mp_SD;
    bool m_isInitialized;
  //  std::vector<Shape*> m_shapes;
    float firstA,lastA;
    uint8_t getAlpha(int pos,size_t total);
    QTimer* m_timer;
    int m_timerId;
};


#endif // DRAWAREA_H
