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
