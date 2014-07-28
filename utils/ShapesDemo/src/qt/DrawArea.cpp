/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
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

void DrawArea::add_ContentFilter()
{
    ContentFilterSelector* a = new ContentFilterSelector(this);
//    QWidget* w = new QWidget(this);
//    w->setObjectName("cfilter");
//    QRect rect(10,10,100,100);
//    w->setGeometry(rect);
//    QPalette pal = w->palette();
//    QBrush brush(Qt::gray,Qt::BDiagPattern);
//    pal.setBrush(QPalette::All,QPalette::Window,brush);
//    pal.setBrush(QPalette::All,QPalette::Base,brush);
//    w->setAutoFillBackground(true);
//    w->setPalette(pal);
//    //LAYOUT
//    QVBoxLayout* layout = new QVBoxLayout(w);
//    layout->setContentsMargins(QMargins());
//    layout->setSpacing(0);
//    QFrame* fr = new QFrame(w);
//    fr->setObjectName("cfilter_frame");
//    QRect rect2(0,0,100,100);
//    fr->setGeometry(rect2);
//    fr->setStyleSheet("border: 1px inset gray;");
//    fr->setVisible(true);
//   // layout->addWidget(fr);
//    layout->addWidget(new QSizeGrip(w), 0, Qt::AlignBottom | Qt::AlignRight);


//    //w->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);


//    w->show();
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
            QMutexLocker(&(*it)->m_mutex);
            //DRAW CONTENT FILTER IF EXISTS:
            if((*it)->m_filter.m_useFilter)
            {
                painter->save();
                m_pen.setColor(SD_QT_GRAY);
                m_pen.setStyle(Qt::SolidLine);
                m_brush.setStyle(Qt::BDiagPattern);
                m_brush.setColor(SD_QT_GRAY);
                painter->setBrush(m_brush);
                painter->setPen(m_pen);
                QRect rect((*it)->m_filter.m_minX,
                           (*it)->m_filter.m_minY,
                           ((*it)->m_filter.m_maxX-(*it)->m_filter.m_minX),
                           ((*it)->m_filter.m_maxY-(*it)->m_filter.m_minY));
                painter->drawRect(rect);
                painter->restore();
            }
            if((*it)->hasReceived)
            {
                // cout << "OK"<<std::flush;
                for(std::vector<std::list<ShapeType>>::iterator vit = (*it)->m_drawShape.m_shapeHistory.begin();
                    vit!=(*it)->m_drawShape.m_shapeHistory.end();++vit)
                {
                    size_t total = vit->size();
                    int index = 0;
                    if(vit->begin()->m_writerGuid != c_Guid_Unknown)
                    {
                        for(std::list<ShapeType>::reverse_iterator sit = vit->rbegin();
                            sit!=vit->rend();++sit)
                        {
                            paintShape(painter,(*it)->m_drawShape.m_type,*sit,getAlpha(index,total),true);
                            ++index;
                        }
                    }
                }
            }
        }
        for(std::vector<ShapePublisher*>::iterator it = mp_SD->m_publishers.begin();
            it!=mp_SD->m_publishers.end();++it)
        {
            QMutexLocker(&(*it)->m_mutex);
            if((*it)->isInitialized)
            {
                // cout << "DrawArea locking PUB: "<<std::flush;
                //cout << "OK"<<std::flush;
                paintShape(painter,(*it)->m_drawShape.m_type,(*it)->m_drawShape.m_mainShape,255);
                //cout << "UNLOCKING PUB"<<endl;
            }
        }
    }
}



void DrawArea::paintShape(QPainter* painter,TYPESHAPE type,ShapeType& shape,uint8_t alpha,bool isHistory)
{
    painter->save();
    m_pen.setColor(SD_QT_BLACK);
    if(isHistory)
        m_pen.setStyle(Qt::DotLine);
    else
        m_pen.setStyle(Qt::SolidLine);
    painter->setPen(m_pen);
    QColor auxc = getColorFromShapeType(shape);
    auxc.setAlpha(alpha);
    m_brush.setColor(auxc);
    m_brush.setStyle(Qt::SolidPattern);
    painter->setBrush(m_brush);
    switch(type)
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


void DrawArea::setShapesDemo(ShapesDemo*SD)
{
    if(SD!=NULL)
    {
        mp_SD = SD;
        m_isInitialized = true;
    }
}
