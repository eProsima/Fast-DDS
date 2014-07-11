/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"


namespace Ui {
class MainWindow;
}

class UpdateThread;
class QStandardItemModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    ShapesDemo* getShapesDemo(){
        return &m_shapesDemo;
    }

    void writeNewSamples();

    void updateInterval(uint32_t ms);

   void quitThreads();

   void addPublisherToTable(ShapePublisher* spub);
   void addSubscriberToTable(ShapeSubscriber* ssub);

private slots:
    void on_bt_publish_clicked();

    void on_bt_subscribe_clicked();

    void on_actionPreferences_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    ShapesDemo m_shapesDemo;
    UpdateThread* mp_writeThread;
    QStandardItemModel* m_pubsub;
};

#endif // MAINWINDOW_H
