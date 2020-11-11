#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent, MainWindow* tmp_mw) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    this->mw = tmp_mw;

    this->process_passwd = new QCryptographicHash(QCryptographicHash::Sha3_256);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnButttonClicked()));

    Resource::my_memset(&tmp_login_message, sizeof(login_message_t));
}

login::~login()
{
    delete ui;
    delete process_passwd;
}

void login::OnButttonClicked(void){
    QString tmp_string;
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

    if(socket.state() != QAbstractSocket::ConnectedState)
        socket.connectToHost(SERVER_ADDR, SERVER_PORT);
    if(!socket.waitForConnected(3000)){
        QMessageBox::critical(NULL, "错误", "无法连接服务器");
        return;
    }
    connect(&socket, SIGNAL(disconnected()), this, SLOT(OnConnectionClosed()));


    socket.write((const char*)&tmp_login_message, sizeof(login_message_t));
    if(!socket.waitForReadyRead(6000)){
         QMessageBox::critical(NULL, "错误", "与服务器通信失败");
         return;
    }

    socket.read(&data_received, 1);
    if(data_received == 0 || 1){
        //QMessageBox::information(NULL, "恭喜", "连接成功");

        mw->init_data(&socket, user_name);
        this->hide();
        mw->show();

    }else if(data_received == -1){
        QMessageBox::critical(NULL, "警告", "用户名不存在");
    }else if(data_received == -2){
        QMessageBox::critical(NULL, "警告", "密码错误");
    }
}

void login::OnConnectionClosed(void){
    QMessageBox::critical(NULL, "错误", "无法连接服务器");
}
