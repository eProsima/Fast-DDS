/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
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

class MainWindow;


class UpdateThread: public QThread
{
     Q_OBJECT
public:
    explicit UpdateThread(QObject* parent = 0);
    ~UpdateThread();

    void setMainW(MainWindow* mw);


private slots:
    void updateAll(void);
protected:
    void run(void);
private:
    QTimer* m_timer;
    MainWindow* m_mainW;
};



#endif /* UPDATETHREAD_H_ */
