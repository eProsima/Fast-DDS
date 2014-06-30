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
#include "ui_mainwindow.h"
#include "eprosimashapesdemo/qt/UpdateThread.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mp_updateThread(NULL)
{
    ui->setupUi(this);
    ui->areaDraw->setShapesDemo(this->getShapesDemo());
    QSpinBox* qs = ui->group_Options->findChild<QSpinBox*>("spin_domainId");
    on_spin_domainId_valueChanged(qs->value());
    cout << "1"<<endl;
    mp_updateThread = new UpdateThread(this);
//    cout << "1"<<endl;
    mp_updateThread->setMainW(this);
//    cout << "1"<<endl;
    //mp_updateThread->start();
    mp_updateThread->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_push_Start_clicked()
{
    this->m_shapesDemo.init();
    mp_updateThread->start();
}

void MainWindow::on_push_Stop_clicked()
{
    this->m_shapesDemo.stop();
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

void MainWindow::on_spin_domainId_valueChanged(int arg1)
{
    m_shapesDemo.setDomainId(arg1);
}

void MainWindow::updateDrawArea()
{
    this->m_shapesDemo.moveAllShapes();
    this->m_shapesDemo.writeAll();
    ui->areaDraw->update();
}
