/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimashapesdemo/qt/subscribedialog.h"
#include "ui_subscribedialog.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"
#include "eprosimashapesdemo/shapesdemo/ShapeSubscriber.h"

#include "fastrtps/utils/TimeConversion.h"


#include <QIntValidator>
#include <QMessageBox>


SubscribeDialog::SubscribeDialog(ShapesDemo* psd,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubscribeDialog),
    mp_sd(psd)
{
    ui->setupUi(this);

    //    ui->lineEdit_maxX->setValidator(new QIntValidator(this));
    //    ui->lineEdit_maxY->setValidator(new QIntValidator(this));
    //    ui->lineEdit_minX->setValidator(new QIntValidator(this));
    //    ui->lineEdit_minY->setValidator(new QIntValidator(this));
    ui->lineEdit_TimeBasedFilter->setValidator(new QIntValidator(this));
    setAttribute ( Qt::WA_DeleteOnClose, true );
}

SubscribeDialog::~SubscribeDialog()
{

    delete ui;
}

void SubscribeDialog::on_buttonBox_accepted()
{
    ShapeSubscriber* SSub = new ShapeSubscriber(this->mp_sd->getParticipant());


    //SHAPE/TOPIC:
    if(this->ui->combo_Shape->currentText() == QString("Square"))
    {
        SSub->m_shapeType = SQUARE;
        SSub->m_attributes.topic.topicName = "Square";
    }
    else if(this->ui->combo_Shape->currentText() == QString("Triangle"))
    {
        SSub->m_shapeType= TRIANGLE;
        SSub->m_attributes.topic.topicName = "Triangle";
    }
    else if(this->ui->combo_Shape->currentText() == QString("Circle"))
    {
        SSub->m_shapeType = CIRCLE;
        SSub->m_attributes.topic.topicName = "Circle";
    }
    SSub->m_attributes.topic.topicDataType = "ShapeType";
    SSub->m_attributes.topic.topicKind = WITH_KEY;

    //History:
    SSub->m_attributes.expectsInlineQos = false;
    SSub->m_attributes.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    SSub->m_attributes.topic.historyQos.depth = this->ui->spin_HistoryQos->value();
    SSub->m_shapeHistory.m_history_depth = this->ui->spin_HistoryQos->value();
    SSub->m_attributes.topic.resourceLimitsQos.max_instances = 20;
    SSub->m_attributes.topic.resourceLimitsQos.max_samples_per_instance = this->ui->spin_HistoryQos->value();
    SSub->m_attributes.topic.resourceLimitsQos.max_samples = this->ui->spin_HistoryQos->value()*20;

    //SSub->m_attributes.qos.m_durabilityService.hasChanged = true;
    //SSub->m_attributes.qos.m_timeBasedFilter.hasChanged = true;
    SSub->m_attributes.qos.m_presentation.hasChanged = true;

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
    case 1:
    {
        SSub->m_attributes.qos.m_ownership.kind = EXCLUSIVE_OWNERSHIP_QOS;
        SSub->m_shapeHistory.m_isExclusiveOwnership = true;
        break;
    }
    }
    //PARTITIONS:
    if(this->ui->checkBox_Asterisk->isChecked())
        SSub->m_attributes.qos.m_partition.push_back("*");
    if(this->ui->checkBox_A->isChecked())
        SSub->m_attributes.qos.m_partition.push_back("A");
    if(this->ui->checkBox_B->isChecked())
        SSub->m_attributes.qos.m_partition.push_back("B");
    if(this->ui->checkBox_C->isChecked())
        SSub->m_attributes.qos.m_partition.push_back("C");
    if(this->ui->checkBox_D->isChecked())
        SSub->m_attributes.qos.m_partition.push_back("D");
    //Time Filter
    if(this->ui->checkBox_timeBasedFilter->isChecked())
    {
        SSub->m_shapeHistory.m_filter.m_useTimeFilter = true;
        if(this->ui->lineEdit_TimeBasedFilter->text()=="INF")
        {
            //pWarning("Setting TimeBasedFilter as Infinite should be avoided, putting 0 instead"<<endl);
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
        SSub->m_shapeHistory.m_filter.m_minimumSeparation = SSub->m_attributes.qos.m_timeBasedFilter.minimum_separation;
    }

    //COntent Filter:
    if(this->ui->checkBox_contentBasedFilter->isChecked())
    {
        SSub->m_shapeHistory.m_filter.m_useContentFilter = true;
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




void SubscribeDialog::on_checkBox_reliable_toggled(bool checked)
{
    if(!checked)
    {
        if(this->ui->comboBox_ownership->currentIndex()==1)
        {
            QMessageBox msgBox;
            msgBox.setText("EXCLUSIVE OWNERSHIP only available with Reliable subscribers");
            msgBox.exec();
        }
        this->ui->comboBox_ownership->setCurrentIndex(0);
    }
}

void SubscribeDialog::on_checkBox_timeBasedFilter_clicked(bool checked)
{
    this->ui->lineEdit_TimeBasedFilter->setEnabled(checked);
}


