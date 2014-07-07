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
#include "eprosimashapesdemo/shapesdemo/ShapePublisher.h"


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
    ShapePublisher* SP = new ShapePublisher(this->mp_sd->getParticipant());
       //Get the different elements
    //ShapeAttributes
    setShapeAttributes(SP);

    //SHAPE/TOPIC:
    if(this->ui->combo_Shape->currentText() == QString("Square"))
    {
        SP->m_shape.m_type = SQUARE;
        SP->m_attributes.topic.topicName = "Square";
    }
    else if(this->ui->combo_Shape->currentText() == QString("Triangle"))
    {
        SP->m_shape.m_type = TRIANGLE;
        SP->m_attributes.topic.topicName = "Triangle";
    }
    else if(this->ui->combo_Shape->currentText() == QString("Circle"))
    {
        SP->m_shape.m_type = CIRCLE;
        SP->m_attributes.topic.topicName = "Circle";
    }
    SP->m_attributes.topic.topicDataType = "ShapeType";
    SP->m_attributes.topic.topicKind = WITH_KEY;

    //History:
    SP->m_attributes.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    SP->m_attributes.topic.historyQos.depth = this->ui->spin_HistoryQos->value();
    //Reliability
    if(this->ui->checkBox_reliable->isChecked())
        SP->m_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    //LIVELINESS:
   // cout << "LIVELINESS "<<this->ui->comboBox_liveliness->currentIndex()<<endl;
   if(this->ui->comboBox_liveliness->currentIndex() == 0)
       SP->m_attributes.qos.m_liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
   if(this->ui->comboBox_liveliness->currentIndex() == 1)
       SP->m_attributes.qos.m_liveliness.kind = MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
   if(this->ui->comboBox_liveliness->currentIndex() == 2)
       SP->m_attributes.qos.m_liveliness.kind = MANUAL_BY_TOPIC_LIVELINESS_QOS;

   if(this->ui->lineEdit_leaseDuration->text()=="INF")
       SP->m_attributes.qos.m_liveliness.lease_duration = c_TimeInfinite;
   else
   {
        QString value = this->ui->lineEdit_leaseDuration->text();
        if(value.toInt()>0)
            SP->m_attributes.qos.m_liveliness.lease_duration = MilliSec2Time_t(value.toInt());
   }
    //PARTITIONS:




    if(SP->initPublisher())
     this->mp_sd->addPublisher(SP);



}

void PublishDialog::setShapeAttributes(ShapePublisher* SP)
{
    //COLOR:
    if(this->ui->combo_Color->currentText() == QString("PURPLE"))
        SP->m_shape.m_mainShape.setColor(SD_PURPLE);
    else if(this->ui->combo_Color->currentText() == QString("BLUE"))
        SP->m_shape.m_mainShape.setColor(SD_BLUE);
    else if(this->ui->combo_Color->currentText() == QString("RED"))
        SP->m_shape.m_mainShape.setColor(SD_RED);
    else if(this->ui->combo_Color->currentText() == QString("GREEN"))
        SP->m_shape.m_mainShape.setColor(SD_GREEN);
    else if(this->ui->combo_Color->currentText() == QString("YELLOW"))
        SP->m_shape.m_mainShape.setColor(SD_YELLOW);
    else if(this->ui->combo_Color->currentText() == QString("CYAN"))
        SP->m_shape.m_mainShape.setColor(SD_CYAN);
    else if(this->ui->combo_Color->currentText() == QString("MAGENTA"))
        SP->m_shape.m_mainShape.setColor(SD_MAGENTA);
    else if(this->ui->combo_Color->currentText() == QString("ORANGE"))
        SP->m_shape.m_mainShape.setColor(SD_ORANGE);
    //SIZE:
    SP->m_shape.m_mainShape.m_size = this->ui->spin_Size->value();
    //POSITION IS RANDOM:
    SP->m_shape.m_mainShape.m_x = this->mp_sd->getRandomX(SP->m_shape.m_mainShape.m_size);
    SP->m_shape.m_mainShape.m_y = this->mp_sd->getRandomY(SP->m_shape.m_mainShape.m_size);

}
