#include "mainwindow.h"
#include "login.h"

#include <QApplication>
#include <QTcpSocket>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTcpSocket socket;
    volatile char tag = -3;

    //login l(nullptr, &socket, &tag);
    //l.show();

    MainWindow w;
    w.show();
    return a.exec();
}
