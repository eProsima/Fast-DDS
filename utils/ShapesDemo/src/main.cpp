#include "eprosimashapesdemo/qt/mainwindow.h"
#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/utils/eClock.h"
#include <QApplication>
//#include <stdafx.h>
#include "eprosimashapesdemo/utils/md5.h"


int main(int argc, char *argv[])
{
    //RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
    Log::logFileName("log.txt");
    Log::setVerbosity(VERB_WARNING);
    Log::setCategoryVerbosity(RTPS_EDP,VERB_INFO);
    Log::setCategoryVerbosity(RTPS_PDP,VERB_INFO);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    a.exec();
    DomainRTPSParticipant::stopAll();
    return 0;
}
