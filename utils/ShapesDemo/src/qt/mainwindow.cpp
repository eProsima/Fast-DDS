/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimashapesdemo/qt/mainwindow.h"
#include "eprosimashapesdemo/qt/publishdialog.h"
#include "eprosimashapesdemo/qt/subscribedialog.h"
#include "eprosimashapesdemo/qt/optionsdialog.h"
#include "ui_mainwindow.h"
#include "eprosimashapesdemo/qt/UpdateThread.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_shapesDemo(this),
    mp_drawThread(NULL),
    mp_writeThread(NULL)
{
    ui->setupUi(this);
    ui->areaDraw->setShapesDemo(this->getShapesDemo());

    mp_drawThread = new UpdateThread(this,0);
    mp_drawThread->setMainW(this);
    mp_drawThread->updateInterval(150);
    mp_drawThread->start();
    mp_writeThread = new UpdateThread(this,1);
    mp_writeThread->setMainW(this);
    mp_writeThread->start();
    this->m_shapesDemo.init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_push_Start_clicked()
{
    this->m_shapesDemo.init();
    //mp_updateThread->start();
}

void MainWindow::on_push_Stop_clicked()
{
    this->m_shapesDemo.stop();
    update();
}


void MainWindow::on_bt_publish_clicked()
{
    PublishDialog* pd = new PublishDialog(this->getShapesDemo(),this);
    pd->show();
}



void MainWindow::on_bt_subscribe_clicked()
{
    SubscribeDialog* pd = new SubscribeDialog(this->getShapesDemo(),this);
    pd->show();
}


void MainWindow::updateDrawArea()
{
     //   cout << "Updating All "<<endl;
//    this->m_shapesDemo.moveAllShapes();
//    this->m_shapesDemo.writeAll();
    ui->areaDraw->update();
}

void MainWindow::writeNewSamples()
{
    this->m_shapesDemo.moveAllShapes();
    this->m_shapesDemo.writeAll();
    //ui->areaDraw->update();
}

void MainWindow::on_actionPreferences_triggered()
{
    OptionsDialog* od = new OptionsDialog(this->getShapesDemo(),this);
    od->show();
}

 void MainWindow::updateInterval(uint32_t ms)
 {
     this->mp_writeThread->updateInterval(ms);
 }
