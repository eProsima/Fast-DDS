#include "eprosimashapesdemo/qt/optionsdialog.h"
#include "ui_optionsdialog.h"
#include "eprosimashapesdemo/shapesdemo/ShapesDemo.h"

OptionsDialog::OptionsDialog(ShapesDemo* psd,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
     mp_sd(psd)
{
    ui->setupUi(this);
    ShapesDemoOptions opt = this->mp_sd->getOptions();
    this->ui->spin_domainId->setValue(opt.m_domainId);
    if(mp_sd->isInitialized())
        this->ui->spin_domainId->setEnabled(false);
    this->ui->spin_updateInterval->setValue(opt.m_updateIntervalMs);
    this->ui->horizontalSlider_speed->setValue(opt.m_movementSpeed);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_OptionsDialog_accepted()
{
    ShapesDemoOptions opt;
    opt.m_domainId = this->ui->spin_domainId->value();
    opt.m_movementSpeed = this->ui->horizontalSlider_speed->value();
    opt.m_updateIntervalMs = this->ui->spin_updateInterval->value();
    mp_sd->setOptions(opt);
}
