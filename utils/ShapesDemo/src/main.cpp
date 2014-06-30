#include "eprosimashapesdemo/qt/mainwindow.h"
#include "eprosimartps/utils/RTPSLog.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);
    QApplication a(argc, argv);

    cout << "Application startes"<<endl;

    MainWindow w;
    cout << "3"<<endl;
//    Q_DebugStream cout(std::cout,a.);
//    Q_DebugStream cerr(std::cerr,QDebug());
    w.show();
    cout << "4"<<endl;

    return a.exec();
}
