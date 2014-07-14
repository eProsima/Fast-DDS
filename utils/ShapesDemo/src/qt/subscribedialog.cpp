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


SubscribeDialog::SubscribeDialog(ShapesDemo* psd,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubscribeDialog),
    mp_sd(psd)
{
    ui->setupUi(this);
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
    cout << "Durability INDEX: "<< this->ui->comboBox_durability->currentIndex() << endl;
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
