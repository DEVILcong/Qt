#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent, MainWindow* tmp_mw, QSslSocket* tmp_socket_ptr) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    this->mw = tmp_mw;
    this->socket_ptr = tmp_socket_ptr;

    this->process_passwd = new QCryptographicHash(QCryptographicHash::Sha3_256);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnButttonClicked()));
    connect(socket_ptr, SIGNAL(disconnected()), this, SLOT(OnConnectionClosed()));

    connect(socket_ptr, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), [=](const QList<QSslError> &errors){
        foreach(QSslError tmp_error, errors){
            qDebug() << tmp_error.errorString() << '\n';
        }
    });

    Resource::my_memset(&tmp_login_message, sizeof(login_message_t));
}

login::~login()
{
    delete ui;
    delete process_passwd;
}

void login::OnButttonClicked(void){
    QString tmp_string;
    QStringList cert_commen_name;
    QStringList cert_email_addr;
    QSslCertificate peer_cert;

    QByteArray tmp_byte_array;
    int tmp_pos = 0;
    char data_received = 0;

    QString user_name = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();

    QList<QString>::iterator i;
    for(i = Resource::forbid_char_in_password.begin(); i!= Resource::forbid_char_in_password.end(); ++i){
        tmp_string = *i;
        tmp_pos = user_name.indexOf(tmp_string);
        tmp_pos *= password.indexOf(tmp_string);

        if(tmp_pos < 0){
            QMessageBox::critical(NULL, "警告", "用户名或密码中含有非法字符");
            return;
        }
    }

    this->process_passwd->reset();
    tmp_byte_array = password.toLocal8Bit();
    this->process_passwd->addData(tmp_byte_array, tmp_byte_array.size());
    tmp_byte_array = this->process_passwd->result();
    tmp_byte_array = tmp_byte_array.toBase64();

    Resource::my_memset(&tmp_login_message, sizeof(login_message_t));
    tmp_login_message.type = 'L';
    tmp_login_message.none = 'N';
    Resource::my_memcpy(user_name.toLocal8Bit().constData(), tmp_login_message.name, user_name.toLocal8Bit().size());
    Resource::my_memcpy(tmp_byte_array.constData(), tmp_login_message.pass, tmp_byte_array.size());

    //qDebug() << tmp_login_message.name << '\n';
    //qDebug() << tmp_login_message.pass << '\n';

    if(socket_ptr->state() != QAbstractSocket::ConnectedState)
        socket_ptr->connectToHostEncrypted(SERVER_ADDR, SERVER_PORT);
    if(!socket_ptr->waitForConnected(3000)){
        QMessageBox::critical(NULL, "错误", "无法连接服务器");
        return;
    }
    if(!socket_ptr->waitForEncrypted(3000)){
        return;
    }

    peer_cert = socket_ptr->peerCertificate();
    cert_commen_name = peer_cert.subjectInfo(QSslCertificate::CommonName);
    cert_email_addr = peer_cert.subjectInfo(QSslCertificate::EmailAddress);
    if(cert_commen_name.at(0) != QString(SERVER_CERT_COMMEN_NAME) || cert_email_addr.at(0) != QString(SERVER_CERT_EMAIL_ADDR)){
        QMessageBox::critical(NULL, "错误", "服务器身份错误");
        socket_ptr->close();
        return;
    }

    socket_ptr->write((const char*)&tmp_login_message, sizeof(login_message_t));
    if(!socket_ptr->waitForReadyRead(6000)){
         QMessageBox::critical(NULL, "错误", "与服务器通信失败");
         return;
    }

    socket_ptr->read(&data_received, 1);
    if(data_received == 0){
        mw->init_data(socket_ptr, user_name);
        this->hide();
        mw->show();
    }else if(data_received == -1){
        QMessageBox::critical(NULL, "警告", "密码错误");
    }
}

void login::OnConnectionClosed(void){
    QMessageBox::critical(NULL, "错误", "无法连接服务器");
}

void login::OnSslError(const QList<QSslError>& ssl_errors){
    QString tmp_string;
    foreach(QSslError tmp_error, ssl_errors){
        tmp_string += tmp_error.errorString();
        tmp_string += "\n";
    }

    QMessageBox::critical(NULL, "错误", tmp_string);
}
