/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file UpdateThread.h
 *
 */

#ifndef UPDATETHREAD_H_
#define UPDATETHREAD_H_

#include <QThread>
#include <QTimer>
#include <cstdint>

#include <iostream>
using namespace std;

class MainWindow;


class UpdateThread: public QThread
{
     Q_OBJECT
public:
    explicit UpdateThread(QObject* parent = 0,uint32_t threadN=200);
    ~UpdateThread();

    void setMainW(MainWindow* mw);
    void updateInterval(uint32_t interval)
    {
      //  cout << "UPDATE: Thread ID: "<< this->thread()->currentThreadId()<<endl;
        m_interval = interval;
        m_hasIntervalChanged = true;
    }

private slots:
    void updateAll(void);
protected:
    void run(void);
private:
    QTimer* m_timer;
    MainWindow* m_mainW;
    uint32_t m_interval;
    uint32_t m_threadNumber;
    bool m_hasIntervalChanged;
};



#endif /* UPDATETHREAD_H_ */
