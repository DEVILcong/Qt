#include "mainwindow.h"
#include "login.h"

#include <QApplication>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslKey>
#include <QFile>
#include <QList>

//QTcpSocket socket;
//QString client_name;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QFile pub_key_file(PUB_KEY_FILE);
//    QFile pri_key_file(PRI_KEY_FILE);
//    QFile ca_cert_file(CA_CERT);

    QSslSocket ssl_socket;
//    QList<QSslError> ignore_ssl_errors;
//    QList<QSslCertificate> ssl_certs;

//    ssl_certs.push_back(QSslCertificate(&pub_key_file));
//    ssl_certs.push_back(QSslCertificate(&ca_cert_file));

//    ssl_socket.setLocalCertificateChain(ssl_certs);
//    ssl_socket.setPrivateKey(QSslKey(&pri_key_file));
//    ssl_socket.setPeerVerifyName(SERVER_HOST_NAME);

    ssl_socket.setPeerVerifyMode(QSslSocket::QueryPeer);

    MainWindow w(nullptr);
    login l(nullptr, &w, &ssl_socket);
    l.show();

    return a.exec();
}
