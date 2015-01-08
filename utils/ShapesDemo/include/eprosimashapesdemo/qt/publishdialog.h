/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#ifndef PUBLISHDIALOG_H
#define PUBLISHDIALOG_H

#include <QDialog>


namespace Ui {
class PublishDialog;
}

class ShapesDemo;
class ShapePublisher;

class PublishDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PublishDialog(ShapesDemo* psd,QWidget *parent = 0);
    ~PublishDialog();

private slots:

    void on_button_OkCancel_accepted();

    void on_comboBox_ownership_currentIndexChanged(int index);

private:
    Ui::PublishDialog *ui;
    ShapesDemo* mp_sd;

    void setShapeAttributes(ShapePublisher* SP);
};

#endif // PUBLISHDIALOG_H
