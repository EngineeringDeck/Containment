#include <QApplication>
#include "container.h"

int main(int argc,char *argv[])
{
    QApplication application(argc,argv);
    Container window;
    window.show();
    return application.exec();
}
