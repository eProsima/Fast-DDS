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

    //Reliability
    if(this->ui->checkBox_reliable->isChecked())
        SSub->m_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    //PARTITIONS:

    //LIVELINESS:


    if(SSub->initSubscriber())
     this->mp_sd->addSubscriber(SSub);

}
