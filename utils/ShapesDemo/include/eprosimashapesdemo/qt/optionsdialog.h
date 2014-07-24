/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS ShapesDemo is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class ShapesDemo;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(ShapesDemo* psd,QWidget *parent = 0);
    ~OptionsDialog();

private slots:
    void on_OptionsDialog_accepted();

private:
    Ui::OptionsDialog *ui;
    ShapesDemo* mp_sd;
};

#endif // OPTIONSDIALOG_H
