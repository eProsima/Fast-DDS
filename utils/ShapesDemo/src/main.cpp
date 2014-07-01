#include "eprosimashapesdemo/qt/mainwindow.h"
#include "eprosimartps/utils/RTPSLog.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
