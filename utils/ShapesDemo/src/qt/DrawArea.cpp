/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/qt/DrawArea.h"

#include <QPainter>

DrawArea::DrawArea(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    m_brush.setStyle(Qt::SolidPattern);



    update();
  //  drawShape();
}

DrawArea::~DrawArea()
{

}

QSize DrawArea::sizeHint() const
{
    return QSize(400, 200);
}

QSize DrawArea::minimumSizeHint() const
{
    return QSize(100, 100);
}

void DrawArea::setPen(const QPen &pen)
{
    this->m_pen = pen;
    update();
}

void DrawArea::setBrush(const QBrush &brush)
{
    this->m_brush = brush;
    update();
}

void DrawArea::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    m_shape.define(SQUARE,SD_BLUE,100,100,100);
    paintShape(&painter,m_shape);
    m_shape.define(SQUARE,SD_BLUE,110,110,100);
    paintShape(&painter,m_shape);
    m_shape.define(CIRCLE,SD_YELLOW);
    paintShape(&painter,m_shape);
    m_shape.define(TRIANGLE,SD_ORANGE,100,200,70);
    paintShape(&painter,m_shape);
}

void DrawArea::paintShape(QPainter *painter,ShapeType& shape)
{
    painter->save();
    m_pen.setColor(SD_QT_BLACK);
    painter->setPen(m_pen);
    QColor auxc = getColorFromShapeType(shape);
    auxc.setAlpha(100);
    m_brush.setColor(auxc);
    painter->setBrush(m_brush);
    switch(shape.m_type)
    {
    case SQUARE:
    {
        QRect rect(shape.m_x-shape.m_shapesize/2,
                   shape.m_y-shape.m_shapesize/2,
                   shape.m_shapesize,
                   shape.m_shapesize);
        painter->drawRect(rect);
     break;
    }
    case TRIANGLE:
    {
        uint32_t x,y,s;
        x = shape.m_x;
        y = shape.m_y;
        s = shape.m_shapesize;
        double h = 0.5*sqrt(3*pow(s,2));
        QPoint points[3] = {
               QPoint(x-s/2, y+h/2),
               QPoint(x+s/2, y+h/2),
               QPoint(x, y-h/2)
           };
        painter->drawPolygon(points,3);
        break;
    }
    case CIRCLE:
    {
        QRect rect(shape.m_x-shape.m_shapesize/2,
                   shape.m_y-shape.m_shapesize/2,
                   shape.m_shapesize,
                   shape.m_shapesize);
        painter->drawEllipse(rect);
        break;
    }
    }
    painter->restore();

}


 QColor DrawArea::getColorFromShapeType(ShapeType& st)
 {
     switch(st.getColor())
     {
     case SD_PURPLE: return SD_QT_PURPLE;
     case SD_BLUE: return SD_QT_BLUE;
     case SD_RED: return SD_QT_RED;
     case SD_GREEN: return SD_QT_GREEN;
     case SD_YELLOW: return SD_QT_YELLOW;
     case SD_CYAN: return SD_QT_CYAN;
     case SD_MAGENTA: return SD_QT_MAGENTA;
     case SD_ORANGE: return SD_QT_ORANGE;
     }
     return SD_QT_BLUE;
 }
