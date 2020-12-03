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
    this->process_msg_ptr = new ProcessMsg(server_keys[0].key, server_keys[0].iv);

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnButttonClicked()));
    connect(socket_ptr, SIGNAL(disconnected()), this, SLOT(OnConnectionClosed()));

//    connect(socket_ptr, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), [=](const QList<QSslError> &errors){
//        foreach(QSslError tmp_error, errors){
//            qDebug() << tmp_error.errorString() << '\n';
//        }
//    });
}

login::~login()
{
    delete ui;
    delete process_passwd;
    delete process_msg_ptr;
}

void login::OnButttonClicked(void){
    static QString tmp_string;
    static QStringList cert_commen_name;
    static QStringList cert_email_addr;
    static QSslCertificate peer_cert;

    QByteArray tmp_byte_array;
    int tmp_pos = 0;
    char data_received = 0;

    int key_seconds = 0;

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

    tmp_json_obj.insert("type", QJsonValue(LOGIN_PASSWD_TYPE));
    tmp_json_obj.insert("username", QJsonValue(user_name));
    tmp_json_obj.insert("passwd", QJsonValue(QString(tmp_byte_array)));

    tmp_json_docu.setObject(tmp_json_obj);
    tmp_byte_array = tmp_json_docu.toJson(QJsonDocument::Compact);

    key_seconds = QTime::currentTime().second() / (60/AES_SERVER_KEY_NUM);
    this->process_msg_ptr->AES_256_change_key(server_keys[key_seconds].key, server_keys[key_seconds].iv);
    this->process_msg_ptr->AES_256_process(tmp_byte_array.data(), tmp_byte_array.length(), 1);
    if(!this->process_msg_ptr->ifValid()){
        QMessageBox::critical(NULL, "错误", "加密失败");
        return;
    }

    tmp_byte_array = QByteArray(((const char*)this->process_msg_ptr->get_result()), this->process_msg_ptr->get_result_length());
    tmp_json_obj_all_message.insert("info", QJsonValue(QString(tmp_byte_array)));
    tmp_json_obj_all_message.insert("value", QJsonValue(key_seconds));

    tmp_json_docu.setObject(tmp_json_obj_all_message);
    tmp_byte_array = tmp_json_docu.toJson(QJsonDocument::Compact);

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

    socket_ptr->write(tmp_byte_array.data(), tmp_byte_array.length());
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
