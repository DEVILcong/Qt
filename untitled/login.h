#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QByteArray>
#include <QSslSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QDebug>

#include "resource.h"
#include "process_msg.hpp"
#include "mainwindow.h"

extern struct aes_key_item_t server_keys[AES_SERVER_KEY_NUM];

struct login_message_t{
     unsigned char type;
     unsigned char none = 'N';
     char name[25];
     char pass[45];
};

namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr, MainWindow* tmp_mw = nullptr, QSslSocket* tmp_socket_ptr = nullptr);
    ~login();

public slots:
    void OnButttonClicked(void);
    void OnConnectionClosed(void);
    void OnSslError(const QList<QSslError>& ssl_errors);

private:
    Ui::login *ui;
    MainWindow* mw;
    QSslSocket* socket_ptr;
    QCryptographicHash *process_passwd;

    QJsonDocument tmp_json_docu;
    QJsonObject tmp_json_obj;
    QJsonObject tmp_json_obj_all_message;
    ProcessMsg* process_msg_ptr;
};

#endif // LOGIN_H
