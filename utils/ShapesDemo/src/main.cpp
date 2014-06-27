#include "eprosimashapesdemo/qt/mainwindow.h"
#include "eprosimartps/utils/RTPSLog.h"
#include <QApplication>

#include "eprosimashapesdemo/qt/QDebugStream.h"

int main(int argc, char *argv[])
{
    RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
    QApplication a(argc, argv);



    MainWindow w;
//    Q_DebugStream cout(std::cout,a.);
//    Q_DebugStream cerr(std::cerr,QDebug());
    w.show();

    return a.exec();
}
