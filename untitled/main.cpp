#include "mainwindow.h"
#include "login.h"

#include <QApplication>
#include <QTcpSocket>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTcpSocket socket;
    volatile char tag = -3;
    QString client_name;

    login l(nullptr, &socket, &tag, &client_name);
    l.show();

    MainWindow w(nullptr, &socket, &client_name);
    w.show();
    return a.exec();
}
