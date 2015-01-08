/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS ShapesDemo is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class ShapesDemo;
class MainWindow;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(MainWindow* mw,ShapesDemo* psd,QWidget *parent = 0);
    ~OptionsDialog();

private slots:
    void on_OptionsDialog_accepted();


    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();

    void on_spin_domainId_valueChanged(int arg1);

private:
    Ui::OptionsDialog *ui;
    ShapesDemo* mp_sd;
    MainWindow* mp_mw;

    void setEnableState();
};

#endif // OPTIONSDIALOG_H
