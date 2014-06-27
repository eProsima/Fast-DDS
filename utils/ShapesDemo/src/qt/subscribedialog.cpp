#include "include/eprosimashapesdemo/qt/subscribedialog.h"
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
