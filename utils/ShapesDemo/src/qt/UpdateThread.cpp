/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file UpdateThread.cpp
 *
 */

#include "eprosimashapesdemo/qt/UpdateThread.h"
#include "eprosimashapesdemo/qt/mainwindow.h"

#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"


UpdateThread::UpdateThread(QObject *parent, uint32_t threadN):
    QThread(parent),
    m_mainW(NULL),
    m_timer(NULL),
    m_interval(INITIAL_INTERVAL_MS),
    m_threadNumber(threadN),
    m_hasIntervalChanged(false)
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
        QTimer::singleShot(0,this,SLOT(updateAll()));
    }
    //cout << "RUN: Thread ID: "<< this->thread()->currentThreadId()<<endl;
    exec();
}


void UpdateThread::updateAll(void)
{
    if(m_timer == NULL)
    {
        m_timer = new QTimer(this);
        connect(m_timer,SIGNAL(timeout()),this,SLOT(updateAll()));
   // cout << "START: Thread ID: "<< this->thread()->currentThreadId()<<endl;
        m_timer->start(m_interval);
    }

   // cout << "WRITE TIMER: "<< this->m_timer->interval();
    //cout << "WRITE: Thread ID: "<< this->thread()->currentThreadId()<<endl;
    if(this->m_threadNumber == 1)
        m_mainW->writeNewSamples();
    if(m_timer !=NULL && m_hasIntervalChanged)
    {
        //cout << "Changing interval timer"<<endl;
        m_timer->setInterval(m_interval);
        m_hasIntervalChanged = false;
    }

}

 void UpdateThread::setMainW(MainWindow* mw)
{
    m_mainW = mw;
}
