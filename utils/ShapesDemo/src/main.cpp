#include "eprosimashapesdemo/qt/mainwindow.h"
#include "eprosimartps/utils/RTPSLog.h"
#include <QApplication>
//#include <stdafx.h>
#include "eprosimashapesdemo/utils/md5.h"

int main(int argc, char *argv[])
{
    RTPSLog::setVerbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL);


//    //TESTS to get the key:
//    //COLOR PURPLE:
//    char cdrcolor[11];
//    cdrcolor[0] = 0x00;
//    cdrcolor[1] = 0x0;
//    cdrcolor[2] = 0x0;
//    cdrcolor[3] = 0x07;
//    cdrcolor[4] = 'P';
//    cdrcolor[5] = 'U';
//    cdrcolor[6] = 'R';
//    cdrcolor[7] = 'P';
//    cdrcolor[8] = 'L';
//    cdrcolor[9] = 'E';
//    cdrcolor[10] = 0x0;
//   // cdrcolor[11] = 0x0;


//    cout << std::hex;
//    for(int i = 0;i<12;++i)
//        cout << std::hex<<(int)cdrcolor[i];
//    cout << endl;
//    cout <<std::hex<< md5(cdrcolor)<<endl;
//     MD5 m =MD5();
//     m.update(cdrcolor,11);
//     m.finalize();
//     cout << m.hexdigest()<<endl;

//     return 0;


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
