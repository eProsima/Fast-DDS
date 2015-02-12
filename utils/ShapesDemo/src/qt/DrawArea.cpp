/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/qt/DrawArea.h"

#include "eprosimashapesdemo/qt/ContentFilterSelector.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/Shape.h"

#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"

#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QSizeGrip>

DrawArea::DrawArea(QWidget *parent)
    : QWidget(parent),
      m_isInitialized(false),
      firstA(10),
      lastA(240)
{
    this->setStyleSheet("QWidget#areaDraw{background-color: rgb(255, 255, 255);background-repeat:none;background-image: url(:/eProsimaLogo.png);background-position:center;}");
    setVisible(true);
    m_brush.setStyle(Qt::SolidPattern);

    m_timerId = startTimer(50);
}

DrawArea::~DrawArea()
{

}

QSize DrawArea::sizeHint() const
{
    return QSize(MAX_DRAW_AREA_X, MAX_DRAW_AREA_Y);
}

QSize DrawArea::minimumSizeHint() const
{
    return QSize(MAX_DRAW_AREA_X, MAX_DRAW_AREA_Y);
}


void DrawArea::paintEvent(QPaintEvent * e/* event */)
{
    QStyleOption opt;
    opt.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    drawShapes(&painter);
}

void DrawArea::addContentFilter(ShapeSubscriber *ssub)
{
    ContentFilterSelector* a = new ContentFilterSelector(this);
    a->assignShapeSubscriber(ssub);
    ssub->assignContentFilterPointer(a);
}

void DrawArea::timerEvent(QTimerEvent* e)
{
    Q_UNUSED(e);
    this->setStyleSheet("QWidget#areaDraw{background-color: rgb(255, 255, 255);background-repeat:none;background-image: url(:/eProsimaLogo.png);background-position:center;}");
    repaint();
}

uint8_t DrawArea::getAlpha(int pos,size_t total)
{
    float inc = (lastA-firstA)/total;
    return (uint8_t)(firstA+inc*(pos+1));
}

void DrawArea::drawShapes(QPainter* painter)
{
    if(m_isInitialized)
    {
        QMutexLocker lock(&mp_SD->m_mutex);
        for(std::vector<ShapeSubscriber*>::iterator it = mp_SD->m_subscribers.begin();
            it!=mp_SD->m_subscribers.end();++it)
        {
            QMutexLocker lock2(&(*it)->m_mutex);
            if((*it)->hasReceived)
            {
                // cout << "OK"<<std::flush;
                for(std::vector<std::list<Shape>>::iterator vit = (*it)->m_shapeHistory.m_history.begin();
                    vit!=(*it)->m_shapeHistory.m_history.end();++vit)
                {
                    size_t total = vit->size();
                    int index = 0;
                    if(vit->begin()->m_writerGuid != c_Guid_Unknown)
                    {
                        for(std::list<Shape>::reverse_iterator sit = vit->rbegin();
                            sit!=vit->rend();++sit)
                        {
                            paintShape(painter,*sit,getAlpha(index,total),true);
                            ++index;
                        }
                    }
                }
            }
        }
        for(std::vector<ShapePublisher*>::iterator it = mp_SD->m_publishers.begin();
            it!=mp_SD->m_publishers.end();++it)
        {
            QMutexLocker lock2(&(*it)->m_mutex);
            if((*it)->hasWritten)
            {
                paintShape(painter,(*it)->m_shape,255);
            }
        }
    }
}



void DrawArea::paintShape(QPainter* painter, Shape &shape, uint8_t alpha, bool isHistory)
{
    painter->save();
    m_pen.setColor(SD_QT_BLACK);
    if(isHistory)
        m_pen.setStyle(Qt::DotLine);
    else
        m_pen.setStyle(Qt::SolidLine);
    painter->setPen(m_pen);
    QColor auxc = SD_COLOR2QColor(shape.m_color);
    auxc.setAlpha(alpha);
    m_brush.setColor(auxc);
    m_brush.setStyle(Qt::SolidPattern);
    painter->setBrush(m_brush);
    switch(shape.m_type)
    {
    case SQUARE:
    {
        QRect rect(shape.m_x-shape.m_size/2,
                   shape.m_y-shape.m_size/2,
                   shape.m_size,
                   shape.m_size);
        painter->drawRect(rect);
        break;
    }
    case TRIANGLE:
    {
        uint32_t x,y,s;
        x = shape.m_x;
        y = shape.m_y;
        s = shape.m_size;
        //double h = 0.5*sqrt(3*pow((double)s,2));
        QPoint points[3] = {
            QPoint(x-s/2, y+s/2),
            QPoint(x+s/2, y+s/2),
            QPoint(x, y-s/2)
        };
        painter->drawPolygon(points,3);
        break;
    }
    case CIRCLE:
    {
        QRect rect(shape.m_x-shape.m_size/2,
                   shape.m_y-shape.m_size/2,
                   shape.m_size,
                   shape.m_size);
        painter->drawEllipse(rect);
        break;
    }
    }
    painter->restore();

}




void DrawArea::setShapesDemo(ShapesDemo*SD)
{
    if(SD!=NULL)
    {
        mp_SD = SD;
        m_isInitialized = true;
    }
}
