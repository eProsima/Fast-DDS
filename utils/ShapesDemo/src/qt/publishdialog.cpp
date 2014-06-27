/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimashapesdemo/qt/publishdialog.h"
#include "ui_publishdialog.h"
#include "eprosimashapesdemo/shapesdemo/ShapeType.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
PublishDialog::PublishDialog(ShapesDemo* psd,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PublishDialog),
    mp_sd(psd)
{
    ui->setupUi(this);
}

PublishDialog::~PublishDialog()
{
    delete ui;
}



void PublishDialog::on_button_OkCancel_accepted()
{
    //Get the different elements
    ShapeType sh;
    //SHAPE:
    if(this->ui->combo_Shape->currentText() == QString("Square"))
        sh.m_type = SQUARE;
    else if(this->ui->combo_Shape->currentText() == QString("Triangle"))
        sh.m_type = TRIANGLE;
    else if(this->ui->combo_Shape->currentText() == QString("Circle"))
        sh.m_type = CIRCLE;
    //COLOR:
    if(this->ui->combo_Color->currentText() == QString("PURPLE"))
        sh.setColor(SD_PURPLE);
    else if(this->ui->combo_Color->currentText() == QString("BLUE"))
        sh.setColor(SD_BLUE);
    else if(this->ui->combo_Color->currentText() == QString("RED"))
        sh.setColor(SD_RED);
    else if(this->ui->combo_Color->currentText() == QString("GREEN"))
        sh.setColor(SD_GREEN);
    else if(this->ui->combo_Color->currentText() == QString("YELLOW"))
        sh.setColor(SD_YELLOW);
    else if(this->ui->combo_Color->currentText() == QString("CYAN"))
        sh.setColor(SD_CYAN);
    else if(this->ui->combo_Color->currentText() == QString("MAGENTA"))
        sh.setColor(SD_MAGENTA);
    else if(this->ui->combo_Color->currentText() == QString("ORANGE"))
        sh.setColor(SD_ORANGE);
    //SIZE:
    sh.m_size = this->ui->spin_Size->value();
    //PARTITIONS:




}
