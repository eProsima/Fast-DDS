/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimashapesdemo/qt/subscribedialog.h"
#include "ui_subscribedialog.h"
#include "eprosimashapesdemo/shapesdemo/ShapeType.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"

#include "eprosimartps/utils/TimeConversion.h"


#include <QIntValidator>


SubscribeDialog::SubscribeDialog(ShapesDemo* psd,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubscribeDialog),
    mp_sd(psd)
{
    ui->setupUi(this);

    ui->lineEdit_maxX->setValidator(new QIntValidator(this));
    ui->lineEdit_maxY->setValidator(new QIntValidator(this));
    ui->lineEdit_minX->setValidator(new QIntValidator(this));
    ui->lineEdit_minY->setValidator(new QIntValidator(this));
    ui->lineEdit_TimeBasedFilter->setValidator(new QIntValidator(this));
}

SubscribeDialog::~SubscribeDialog()
{
    delete ui;
}

void SubscribeDialog::on_buttonBox_accepted()
{
    ShapeSubscriber* SSub = new ShapeSubscriber(this->mp_sd->getParticipant());

    SSub->m_attributes.expectsInlineQos = false;


    //SHAPE/TOPIC:
    if(this->ui->combo_Shape->currentText() == QString("Square"))
    {
        SSub->m_shape.m_type = SQUARE;
        SSub->m_attributes.topic.topicName = "Square";
    }
    else if(this->ui->combo_Shape->currentText() == QString("Triangle"))
    {
        SSub->m_shape.m_type = TRIANGLE;
        SSub->m_attributes.topic.topicName = "Triangle";
    }
    else if(this->ui->combo_Shape->currentText() == QString("Circle"))
    {
        SSub->m_shape.m_type = CIRCLE;
        SSub->m_attributes.topic.topicName = "Circle";
    }
    SSub->m_attributes.topic.topicDataType = "ShapeType";
    SSub->m_attributes.topic.topicKind = WITH_KEY;

    //History:
    SSub->m_attributes.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    SSub->m_attributes.topic.historyQos.depth = this->ui->spin_HistoryQos->value();
    SSub->m_attributes.topic.resourceLimitsQos.max_instances = 20;
    SSub->m_attributes.topic.resourceLimitsQos.max_samples_per_instance = this->ui->spin_HistoryQos->value();
    SSub->m_attributes.topic.resourceLimitsQos.max_samples = this->ui->spin_HistoryQos->value()*20;

    //Reliability
    if(this->ui->checkBox_reliable->isChecked())
        SSub->m_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    //DURABILITY
   // cout << "Durability INDEX: "<< this->ui->comboBox_durability->currentIndex() << endl;
    switch(this->ui->comboBox_durability->currentIndex())
    {
    case 0: SSub->m_attributes.qos.m_durability.kind = VOLATILE_DURABILITY_QOS; break;
    case 1: SSub->m_attributes.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS; break;
    }
    //Ownership:
    switch(this->ui->comboBox_ownership->currentIndex())
    {
    case 0: SSub->m_attributes.qos.m_ownership.kind = SHARED_OWNERSHIP_QOS; break;
    case 1: SSub->m_attributes.qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS; break;
    }
    //PARTITIONS:
    if(this->ui->checkBox_Asterisk->isChecked())
        SSub->m_attributes.qos.m_partition.names.push_back("*");
    if(this->ui->checkBox_A->isChecked())
        SSub->m_attributes.qos.m_partition.names.push_back("A");
    if(this->ui->checkBox_B->isChecked())
        SSub->m_attributes.qos.m_partition.names.push_back("B");
    if(this->ui->checkBox_C->isChecked())
        SSub->m_attributes.qos.m_partition.names.push_back("C");
    if(this->ui->checkBox_D->isChecked())
        SSub->m_attributes.qos.m_partition.names.push_back("D");
    //Time Filter
    if(this->ui->lineEdit_TimeBasedFilter->text()=="INF")
    {
        pWarning("Setting TimeBasedFilter as Infinite should be avoided"<<endl);
        SSub->m_attributes.qos.m_timeBasedFilter.minimum_separation.seconds = 0;
    }
    else
    {
         QString value = this->ui->lineEdit_TimeBasedFilter->text();
         //cout << "TIME VALUE: "<< value.toStdString() << endl;
         if(value.toInt()>0)
         {
             SSub->m_attributes.qos.m_timeBasedFilter.minimum_separation = TimeConv::MilliSeconds2Time_t(value.toInt());
         }

    }
    //COntent Filter:
    if(this->ui->checkBox_contentBasedFilter->isChecked())
    {
        SSub->m_filter.m_useFilter = true;
        SSub->m_filter.m_maxX = this->ui->lineEdit_maxX->text().toInt();
        SSub->m_filter.m_maxY = this->ui->lineEdit_maxY->text().toInt();
        SSub->m_filter.m_minX = this->ui->lineEdit_minX->text().toInt();
        SSub->m_filter.m_minY = this->ui->lineEdit_minY->text().toInt();
    }
    if(SSub->initSubscriber())
     this->mp_sd->addSubscriber(SSub);
}

void SubscribeDialog::on_comboBox_ownership_currentIndexChanged(int index)
{
    if(index == 1)
    {
        this->ui->checkBox_reliable->setChecked(true);
    }
}

void SubscribeDialog::on_checkBox_contentBasedFilter_toggled(bool checked)
{
    this->ui->lineEdit_minY->setEnabled(checked);
    this->ui->lineEdit_minY->setText(QString("0"));
    this->ui->lineEdit_minX->setEnabled(checked);
    this->ui->lineEdit_minX->setText(QString("0"));
    this->ui->lineEdit_maxX->setEnabled(checked);
    this->ui->lineEdit_maxX->setText(QString("%1").arg(MAX_DRAW_AREA_X));
    this->ui->lineEdit_maxY->setEnabled(checked);
    this->ui->lineEdit_maxY->setText(QString("%1").arg(MAX_DRAW_AREA_Y));
}


void SubscribeDialog::on_lineEdit_minX_editingFinished()
{
    if(this->ui->lineEdit_minX->text().toInt()<0 || this->ui->lineEdit_minX->text().toInt()>=this->ui->lineEdit_maxX->text().toInt())
        this->ui->lineEdit_minX->setText(QString("0"));
}

void SubscribeDialog::on_lineEdit_maxX_editingFinished()
{
    if(this->ui->lineEdit_maxX->text().toInt()>MAX_DRAW_AREA_X || this->ui->lineEdit_maxX->text().toInt()<=this->ui->lineEdit_minX->text().toInt())
        this->ui->lineEdit_maxX->setText(QString("%1").arg(MAX_DRAW_AREA_X));
}

void SubscribeDialog::on_lineEdit_minY_editingFinished()
{
    if(this->ui->lineEdit_minY->text().toInt()<0 || this->ui->lineEdit_minY->text().toInt()>=this->ui->lineEdit_maxY->text().toInt())
        this->ui->lineEdit_minY->setText(QString("0"));
}

void SubscribeDialog::on_lineEdit_maxY_editingFinished()
{
    if(this->ui->lineEdit_maxY->text().toInt()>MAX_DRAW_AREA_Y || this->ui->lineEdit_maxY->text().toInt()<=this->ui->lineEdit_minY->text().toInt())
        this->ui->lineEdit_maxY->setText(QString("%1").arg(MAX_DRAW_AREA_Y));
}
