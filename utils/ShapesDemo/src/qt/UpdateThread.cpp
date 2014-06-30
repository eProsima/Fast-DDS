/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file UpdateThread.cpp
 *
 */

#include "eprosimashapesdemo/qt/UpdateThread.h"
#include "eprosimashapesdemo/qt/mainwindow.h"


UpdateThread::UpdateThread(QObject *parent):
    QThread(parent),
    m_mainW(NULL),
    m_timer(NULL)
{

}

UpdateThread::~UpdateThread()
{
    if(m_timer != NULL)
       delete m_timer;
    quit();
}

void UpdateThread::run(void)
{
    if(m_timer==NULL)
    {
        m_timer = new QTimer(this);
        connect(m_timer,SIGNAL(timeout()),this,SLOT(updateAll()));
    }
    m_timer->start(100);
    exec();
}


void UpdateThread::updateAll(void)
{
    m_mainW->updateDrawArea();
}

 void UpdateThread::setMainW(MainWindow* mw)
{
    m_mainW = mw;
}
