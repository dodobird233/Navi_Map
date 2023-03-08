#include "ex5_mapNavi.h"
#include <QtWidgets/QApplication>
#include <qdesktopwidget.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ex5_mapNavi w;
    w.show();
    return a.exec();
}
