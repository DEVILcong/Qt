#include <QCoreApplication>
#include "mywebsocket.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MyWebsocket tmp_mywebsocket();

    return a.exec();
}
