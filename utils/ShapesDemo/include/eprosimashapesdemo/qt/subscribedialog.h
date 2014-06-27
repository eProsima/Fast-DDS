/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#ifndef SUBSCRIBEDIALOG_H
#define SUBSCRIBEDIALOG_H

#include <QDialog>

namespace Ui {
class SubscribeDialog;
}

class SubscribeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SubscribeDialog(QWidget *parent = 0);
    ~SubscribeDialog();

private:
    Ui::SubscribeDialog *ui;
};

#endif // SUBSCRIBEDIALOG_H
