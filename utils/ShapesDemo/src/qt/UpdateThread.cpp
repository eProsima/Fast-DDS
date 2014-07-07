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


UpdateThread::UpdateThread(QObject *parent, uint32_t threadN):
    QThread(parent),
    m_mainW(NULL),
    m_timer(NULL),
    m_interval(200),
    m_threadNumber(threadN)
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
    m_timer->start(m_interval);
    exec();
}


void UpdateThread::updateAll(void)
{
    if(this->m_threadNumber == 0)
        m_mainW->updateDrawArea();
    else if(this->m_threadNumber == 1)
        m_mainW->writeNewSamples();
}

 void UpdateThread::setMainW(MainWindow* mw)
{
    m_mainW = mw;
}
