/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include "eprosimashapesdemo/qt/subscribedialog.h"
#include "ui_subscribedialog.h"

SubscribeDialog::SubscribeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubscribeDialog)
{
    ui->setupUi(this);
}

SubscribeDialog::~SubscribeDialog()
{
    delete ui;
}
