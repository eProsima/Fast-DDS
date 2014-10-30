#include "eprosimashapesdemo/qt/mainwindow.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
#include <QApplication>
//#include <stdafx.h>
#include "eprosimashapesdemo/utils/md5.h"

int main(int argc, char *argv[])
{
    //RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
    Log::logFileName("log.txt");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    a.exec();
    DomainParticipant::stopAll();
    return 0;
}
