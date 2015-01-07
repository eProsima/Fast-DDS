/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/qt/ContentFilterSelector.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"
#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QSizeGrip>

#include <QMouseEvent>
#include <QPainter>
#include <iostream>
using namespace std;

#define CORNER_MARK_DIM 5
#define FRAME_WIDTH 1

ContentFilterSelector::ContentFilterSelector( QWidget *parent)
    : QWidget(parent),
      m_size_x(100),
      m_size_y(100),
      m_moveFilter(false),
      m_resizeFilter(false),
      mp_ssub(NULL)
{
    ContentFilterSelector* w = this;
    w->setObjectName("cfilter");
    QRect rect(10,10,m_size_x,m_size_y);
    w->setGeometry(rect);
    QPalette pal = w->palette();
    QBrush brush(Qt::gray,Qt::BDiagPattern);
    pal.setBrush(QPalette::All,QPalette::Window,brush);
    pal.setBrush(QPalette::All,QPalette::Base,brush);
    w->setAutoFillBackground(true);
    w->setPalette(pal);
    //LAYOUT
    QVBoxLayout* layout = new QVBoxLayout(w);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    m_frame = new QFrame(w);
    m_frame->setObjectName("cfilter_frame");
    QRect rect2(0,0,100,100);
    m_frame->setGeometry(rect2);
    m_frame->setStyleSheet("border: 1px inset gray;");
    m_frame->setVisible(true);
    // layout->addWidget(fr);

    //w->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);

    w->setMouseTracking(true);
    w->show();

    //SET PEN AND BRUSH FOR THE POINTS
    m_pen.setColor(Qt::black);
    m_brush.setStyle(Qt::SolidPattern);
    m_brush.setColor(Qt::black);
}

ContentFilterSelector::~ContentFilterSelector()
{

}

void ContentFilterSelector::assignShapeSubscriber(ShapeSubscriber* ssub)
{
    mp_ssub = ssub;
    contentFilterChanged();
}

void ContentFilterSelector::contentFilterChanged()
{
    ShapeFilter filt;
    filt.m_minX = this->x();
    filt.m_minY = this->y();
    filt.m_maxX = this->x()+this->width();
    filt.m_maxY = this->y()+this->height();
    mp_ssub->adjustContentFilter(filt);
}

void ContentFilterSelector::mousePressEvent(QMouseEvent* event)
{
    event->accept(); // do not propagate
    if(event->button() == Qt::LeftButton)
    {
        offset = event->pos();
        m_corner = getCorner(event);
        switch (m_corner) {
        case SD_OUTSIDE:  break;
        case SD_CENTER: m_moveFilter = true; break;
        case SD_TOPLEFT:
        case SD_TOPRIGHT:
        case SD_BOTTOMLEFT:
        case SD_BOTTOMRIGHT: m_resizeFilter = true;break;
        }
    }
    else
    {
        m_resizeFilter = false;
        m_moveFilter = false;
    }
}


void ContentFilterSelector::mouseMoveEvent(QMouseEvent* event)
{

    event->accept(); // do not propagate
    if(m_moveFilter)
    {
        QPoint newpos = mapToParent(event->pos() - offset);
        if(newpos.x()+this->width() < MAX_DRAW_AREA_X &&
                newpos.y()+this->height() < MAX_DRAW_AREA_Y &&
                newpos.x()>0 &&
                newpos.y()>0)
        {
            move(newpos);
        }
        else
        {
            offset = event->pos();
        }

    }
    else if(m_resizeFilter)
    {
        switch(m_corner)
        {
        case SD_TOPLEFT:
        {
            break;
        }
        case SD_TOPRIGHT:
        {
            break;
        }
        case SD_BOTTOMLEFT:
        {
            break;
        }
        case SD_BOTTOMRIGHT:
        {
            int dimx = event->pos().x()+FRAME_WIDTH*2+CORNER_MARK_DIM;
            int dimy = event->pos().y()+FRAME_WIDTH*2+CORNER_MARK_DIM;
            if(dimx > 10 && dimy > 10)
            {
                QPoint pos = mapToParent(event->pos());
                if(pos.x()< MAX_DRAW_AREA_X-2*FRAME_WIDTH-CORNER_MARK_DIM &&
                        pos.y()<MAX_DRAW_AREA_Y-2*FRAME_WIDTH-CORNER_MARK_DIM)
                {
                    this->resize(dimx,dimy);
                    m_frame->resize(dimx,dimy);

                }
            }
            break;
        }
        }
    }

}

void ContentFilterSelector::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept(); // do not propagate
    offset = QPoint();
    m_moveFilter = false;
    m_resizeFilter = false;
   contentFilterChanged();
}

void ContentFilterSelector::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QRect dim = this->geometry();
    painter.save();
    painter.setBrush(m_brush);
    painter.setPen(m_pen);
    //cout << "DIMENSIONS: "<< dim.width() << " "<< dim.height() << endl;
    //painter.drawRect(QRect(0+FRAME_WIDTH,0+FRAME_WIDTH,CORNER_MARK_DIM,CORNER_MARK_DIM));
    // painter.drawRect(QRect(dim.width()-CORNER_MARK_DIM-2*FRAME_WIDTH,0+FRAME_WIDTH,CORNER_MARK_DIM,CORNER_MARK_DIM));
    // painter.drawRect(QRect(0+FRAME_WIDTH,dim.height()-2*FRAME_WIDTH-CORNER_MARK_DIM,CORNER_MARK_DIM,CORNER_MARK_DIM));
    painter.drawRect(QRect(dim.width()-2*FRAME_WIDTH-CORNER_MARK_DIM,dim.height()-2*FRAME_WIDTH-CORNER_MARK_DIM,CORNER_MARK_DIM,CORNER_MARK_DIM));
    painter.restore();
}

SD_CONTENTCORNER ContentFilterSelector::getCorner(QMouseEvent* event)
{
    QPoint p = event->pos();
    QRect dim = this->geometry();
   // cout << "POS: "<< p.x() << " "<<p.y() << " DIMENSIONS: "<< dim.width() << " "<<dim.height()<<endl;
    if(p.x()<0 || p.y() < 0 || p.x() > dim.width() || p.y() > dim.height())
        return SD_OUTSIDE; //THIS SHOULD NEVER HAPPEN
    if(p.x() < FRAME_WIDTH+CORNER_MARK_DIM)
    {
        if(p.y() < FRAME_WIDTH+CORNER_MARK_DIM)
            return SD_TOPLEFT;
        if(p.y() > dim.height()-FRAME_WIDTH*2-CORNER_MARK_DIM)
            return SD_BOTTOMLEFT;
    }
    if(p.x() > dim.width()-FRAME_WIDTH*2-CORNER_MARK_DIM)
    {
        if(p.y() < FRAME_WIDTH+CORNER_MARK_DIM)
            return SD_TOPRIGHT;
        if(p.y() > dim.height()-FRAME_WIDTH*2-CORNER_MARK_DIM)
            return SD_BOTTOMRIGHT;
    }
    return SD_CENTER;
}

