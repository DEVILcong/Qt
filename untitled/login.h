#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QByteArray>
#include <QTcpSocket>
#include <QDebug>

#include "resource.h"
#include "mainwindow.h"

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
    explicit login(QWidget *parent = nullptr, MainWindow* tmp_mw = nullptr);
    ~login();

public slots:
    void OnButttonClicked(void);
    void OnConnectionClosed(void);

private:
    Ui::login *ui;
    MainWindow* mw;
    QTcpSocket socket;
    QCryptographicHash *process_passwd;
    login_message_t tmp_login_message;
};

#endif // LOGIN_H
