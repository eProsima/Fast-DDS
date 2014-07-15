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

enum SD_ENDP_TYPE
{
    PUB,
    SUB
};

struct SD_Endpoint
{
    SD_ENDP_TYPE type;
    ShapePublisher* pub;
    ShapeSubscriber* sub;
    int pos;
};

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

    void on_tableEndpoint_customContextMenuRequested(const QPoint &pos);

    void on_actionDelete_Enpoint_triggered();

private:
    Ui::MainWindow *ui;
    ShapesDemo m_shapesDemo;
    UpdateThread* mp_writeThread;
    QStandardItemModel* m_pubsub;
    std::vector<SD_Endpoint> m_pubsub_pointers;
    int m_tableRow;
};

#endif // MAINWINDOW_H
