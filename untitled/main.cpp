#include "mainwindow.h"
#include "login.h"

#include <QApplication>
#include <QTcpSocket>

//QTcpSocket socket;
//QString client_name;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w(nullptr);

    login l(nullptr, &w);
    l.show();

    //w.init_data();
    //w.show();

    return a.exec();
}
