/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef CONTENTFILTERSELECTOR_H
#define CONTENTFILTERSELECTOR_H

#include <QBrush>
#include <QPen>
#include <QWidget>
#include <QTimer>



/**
 * @brief Class ContentFilterSelector, implements content filter selector.
 */
class ContentFilterSelector: public QWidget
{
   // Q_OBJECT
public:
    ContentFilterSelector(QWidget* parent=0);
    virtual ~ContentFilterSelector();

//    QSize minimumSizeHint() const;
//    QSize sizeHint() const;

protected:


private:


private slots:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    QPoint offset;

};


#endif // CONTENTFILTERSELECTOR_H
