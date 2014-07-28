/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#include "eprosimashapesdemo/qt/ContentFilterSelector.h"


#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QSizeGrip>

#include <QMouseEvent>

ContentFilterSelector::ContentFilterSelector(QWidget *parent)
    : QWidget(parent)
{
    ContentFilterSelector* w = this;
    w->setObjectName("cfilter");
    QRect rect(10,10,100,100);
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
    QFrame* fr = new QFrame(w);
    fr->setObjectName("cfilter_frame");
    QRect rect2(0,0,100,100);
    fr->setGeometry(rect2);
    fr->setStyleSheet("border: 1px inset gray;");
    fr->setVisible(true);
    // layout->addWidget(fr);

    //w->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);

    w->setMouseTracking(true);
    w->show();


}

ContentFilterSelector::~ContentFilterSelector()
{

}

//QSize ContentFilterSelector::sizeHint() const
//{
//    return QSize(100, 100);
//}

//QSize ContentFilterArea::minimumSizeHint() const
//{
//    return QSize(100, 100);
//}

void ContentFilterSelector::mousePressEvent(QMouseEvent* event)
{
    event->accept(); // do not propagate
    if (isWindow())
        offset = event->globalPos() - pos();
    else
        offset = event->pos();
}

void ContentFilterSelector::mouseMoveEvent(QMouseEvent* event)
{
    event->accept(); // do not propagate
    if (isWindow())
        move(event->globalPos() - offset);
    else
        move(mapToParent(event->pos() - offset));
}

void ContentFilterSelector::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept(); // do not propagate
    offset = QPoint();
}



//void DrawArea::add_ContentFilter()
//{
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
//}
