/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef DRAWAREA_H
#define DRAWAREA_H

#include <QBrush>
#include <QPen>
#include <QWidget>
#include "eprosimashapesdemo/shapesdemo/ShapeType.h"

class ShapesDemo;

#define SD_QT_COLOR_TRANS 255

const QColor SD_QT_PURPLE = QColor(255,0,255,SD_QT_COLOR_TRANS);
const QColor SD_QT_BLUE = QColor(0,0,255,SD_QT_COLOR_TRANS);
const QColor SD_QT_RED = QColor(255,0,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_GREEN = QColor(0,255,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_YELLOW = QColor(255,255,0,SD_QT_COLOR_TRANS);
const QColor SD_QT_CYAN = QColor(0,176,246,SD_QT_COLOR_TRANS);
const QColor SD_QT_MAGENTA = QColor(245,0,135,SD_QT_COLOR_TRANS);
const QColor SD_QT_ORANGE = QColor(255,130,0,SD_QT_COLOR_TRANS);

const QColor SD_QT_BLACK = QColor(0,0,0,255);

class QPainter;
class Shape;

class DrawArea: public QWidget
{
    Q_OBJECT

public:
    DrawArea(QWidget* parent=0);
    virtual ~DrawArea();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setShapesDemo(ShapesDemo* SD);
    void drawShapes(QPainter*);


protected:
    void paintEvent(QPaintEvent *event);

private:
    QPen m_pen;
    QBrush m_brush;
    ShapeType m_shape;
    QColor getColorFromShapeType(ShapeType& st);
    void paintShape(QPainter*painter, TYPESHAPE type, ShapeType& sh, uint8_t alpha);

    ShapesDemo* mp_SD;
    bool m_isInitialized;
    std::vector<Shape*> m_shapes;

};


#endif // DRAWAREA_H
