#include <QCoreApplication>
#include <signal.h>
#include <unistd.h>
#include "mywebsocket.h"
#include <iostream>

static QCoreApplication *a = nullptr;

void signal_handler(int signum){
    std::cout << "\nINFO: received signal " << signum << std::endl;
    a->quit();
}

int main(int argc, char *argv[])
{
    a = new QCoreApplication(argc, argv);

    struct sigaction newact, oldact;
    newact.sa_handler = signal_handler;
    sigemptyset(&(newact.sa_mask));
    newact.sa_flags = 0;
    sigaction(SIGINT, &newact, &oldact);

    MyWebsocket tmp_mywebsocket;

    a->exec();
    delete a;
    return 0;
}
