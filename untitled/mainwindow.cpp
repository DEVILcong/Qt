#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if_continue_tag = true;
    timer = new QTimer(this);

    socket = nullptr;
    client_name = "";
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_model;
    delete m_delegate;

    delete timer;

    delete tmp_process_msg;
}

void MainWindow::init_data(QSslSocket* tmp_socket, QString& tmp_client_name){
    socket = tmp_socket;
    client_name = tmp_client_name;

    this->setWindowTitle(client_name);

    connect(socket, SIGNAL(disconnected()), this, SLOT(on_connection_lost()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(on_message_arrival()));
    connect(timer, SIGNAL(timeout()), this, SLOT(keep_alive()));
    connect(ui->listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(on_client_clicked(const QModelIndex&)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_send_button_clicked()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(on_filter_button_clicked()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(on_cancel_button_clicked()));

    QFontDatabase database;
    ui->textBrowser->setFont(QFont(database.families(QFontDatabase::SimplifiedChinese)[0], 14));

    m_delegate = new item_delegate(this);
    ui->listView->setSpacing(3);
    ui->listView->setItemDelegate(m_delegate);

    QJsonDocument tmp_json_doc;
    QJsonObject tmp_json_obj;

    tmp_json_obj.insert("receiver", QJsonValue(SERVER_NAME));
    tmp_json_obj.insert("sender", QJsonValue(client_name));
    tmp_json_obj.insert("type", QJsonValue(MSG_TYPE_KEEPALIVE));

    tmp_json_doc.setObject(tmp_json_obj);
    byte_array_keep_alive = tmp_json_doc.toJson(QJsonDocument::Compact);

    tmp_json_obj["type"] = QJsonValue(MSG_TYPE_GET_USER_LIST);
    tmp_json_doc.setObject(tmp_json_obj);
    byte_array_get_user_list = tmp_json_doc.toJson(QJsonDocument::Compact);

    m_model = new QStandardItemModel(this);
    m_proxy_model = new QSortFilterProxyModel(ui->listView);
    m_proxy_model->setSourceModel(m_model);
    m_proxy_model->setFilterRole(Qt::UserRole + 1);
    m_proxy_model->setDynamicSortFilter(true);

    ui->listView->setModel(m_proxy_model);
    ui->listView->setDragEnabled(false);

    tmp_process_msg = new ProcessMsg(client_keys[0].key, client_keys[0].iv);

    timer->start(KEEP_ALIVE_SEND_INTERVAL_SECONDS * 1000);
}

void MainWindow::keep_alive(void){
    QMutexLocker tmp_lock_socket(&socket_mtx);
    socket->write(byte_array_keep_alive.data(), byte_array_keep_alive.length());
}

bool MainWindow::get_user_list(void){
    QMutexLocker tmp_lock_socket(&socket_mtx);

    int tmp_length = socket->write(byte_array_get_user_list.data(), byte_array_get_user_list.length());

    if(tmp_length <= 0)
        return false;
    else
        return true;
}

void MainWindow::refresh_user_list(QJsonArray& tmp_array){
    QStandardItem* tmp_item;
    QString tmp_string;
    m_model->clear();

    for(QJsonArray::iterator i = tmp_array.begin(); i != tmp_array.end(); ++i){
        tmp_string = i->toString();
        if(tmp_string != client_name){
            tmp_item = new QStandardItem;
            tmp_item->setData(QVariant(i->toString()), Qt::UserRole);
            tmp_item->setData(QVariant(0), Qt::UserRole + 1);

            m_model->appendRow(tmp_item);
        }
    }
}

bool MainWindow::send_msg(QByteArray &msg){
    QMutexLocker tmp_locker(&socket_mtx);

    int tmp_length = socket->write(msg.data(), msg.length());

    if(tmp_length <= 0){
        for(int i = 0; i < MSG_SEND_TRIED_TIME; ++i){
            QThread::msleep(MSG_SEND_TRIED_INTERVAL_MSECONDS);
            tmp_length = socket->write(msg.data(), msg.length());
            if(tmp_length >= 0)
                return true;
        }
    }else{
        return true;
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent* event){
    socket->close();
    //lg->close();

    event->accept();
}

void MainWindow::refresh_msg_area(void){
    QMutexLocker tmp_locker_message_buffer(&message_buffer_map_mtx);

    ui->textBrowser->clear();

    item_message_buffer_ptr = &(message_buffer_map[current_client]);

    for(QVector<QString>::iterator i = item_message_buffer_ptr->begin(); i != item_message_buffer_ptr->end(); ++i){
        ui->textBrowser->insertHtml(*i);
    }
}

void MainWindow::on_message_arrival(void){
    QMutexLocker tmp_locker_socket(&socket_mtx);
    QMutexLocker tmp_locker_message_buffer(&message_buffer_map_mtx);
    QMutexLocker tmp_locker_if_continue_tag(&if_continue_mtx);

    static int tmp_length = 0;
    static int key_selector = 0;
    static QByteArray tmp_byte_array;
    static QJsonDocument tmp_json_document;
    static QJsonArray tmp_json_array;
    static QString tmp_string_sender;
    static QString tmp_string_content;
    static QString tmp_string_type;

    static QModelIndexList tmp_model_list;
    static QModelIndex tmp_index;

    static QList<QByteArray> tmp_byte_array_list;

    static char data[RECEIVE_BUFFER_SIZE];
    Resource::my_memset(data, RECEIVE_BUFFER_SIZE);

    Resource::my_memset(data, RECEIVE_BUFFER_SIZE);
    tmp_length = socket->read(data, RECEIVE_BUFFER_SIZE);

    tmp_byte_array = QByteArray(data, tmp_length);
    tmp_byte_array_list = tmp_byte_array.split(MESSAGE_SPLIT);

    for(int i = 0; i < tmp_byte_array_list.size(); ++i){

        tmp_json_document = QJsonDocument::fromJson(tmp_byte_array_list[i]);
        tmp_string_sender = tmp_json_document["sender"].toString();
        tmp_string_type = tmp_json_document["type"].toString();

        if(tmp_string_type == MSG_TYPE_NORMAL){

            tmp_string_content = tmp_json_document["content"].toString();
            tmp_byte_array = tmp_string_content.toLocal8Bit();
            key_selector = tmp_json_document["no"].toInt();

            this->tmp_process_msg->AES_256_change_key(client_keys[key_selector].key, client_keys[key_selector].iv);
            this->tmp_process_msg->AES_256_process(tmp_byte_array.data(), tmp_byte_array.length(), 0);
            if(!this->tmp_process_msg->ifValid()){
                tmp_string_content = QString("--------解密失败-----------");
                continue;
            }else{
                tmp_byte_array = QByteArray((const char*)this->tmp_process_msg->get_result(), this->tmp_process_msg->get_result_length());
                tmp_string_content = QString(tmp_byte_array);
            }

            tmp_locker_message_buffer.relock();
            message_buffer_map[tmp_string_sender].push_back("<b style=\"color:red\">"+ tmp_string_sender + "</b><br>" + tmp_string_content + "<br>");
            tmp_locker_message_buffer.unlock();

            if(current_client == tmp_string_sender){
                ui->textBrowser->insertHtml("<b style=\"color:red\">"+ tmp_string_sender + "</b><br>" + tmp_string_content + "<br>");
            }else{
                tmp_model_list = m_proxy_model->match(m_proxy_model->index(0, 0), Qt::UserRole, QVariant(tmp_string_sender), 1, Qt::MatchExactly);
                foreach(tmp_index, tmp_model_list)
                    m_proxy_model->setData(tmp_index, QVariant(1), Qt::UserRole + 1);
            }

        }else if(tmp_string_type == MSG_TYPE_GET_USER_LIST){
            tmp_json_array = tmp_json_document["content"].toArray();
            refresh_user_list(tmp_json_array);
        }else if(tmp_string_type == MSG_TYPE_ERORR){
            tmp_string_content = tmp_json_document["content"].toString();
            ui->textBrowser->insertHtml("<sup style=\"color:red\">send failed</sup><br>");
        }else{
            continue;
        }
    }
}

void MainWindow::on_client_clicked(const QModelIndex& index){
    QMutexLocker tmp_locker(&message_buffer_map_mtx);
    ui->textBrowser->clear();
    ui->textEdit->clear();

    m_proxy_model->setData(index, QVariant(0), Qt::UserRole + 1);
    current_client = index.data(Qt::UserRole).toString();
    item_message_buffer_ptr = &(message_buffer_map[current_client]);

    for(QVector<QString>::iterator i = item_message_buffer_ptr->begin(); i != item_message_buffer_ptr->end(); ++i){
        ui->textBrowser->insertHtml(*i);
    }
}

void MainWindow::on_connection_lost(void){
    QMessageBox::critical(NULL, "警告", "服务器连接中断");
}

void MainWindow::on_send_button_clicked(void){
    QJsonObject tmp_json_object;
    QJsonDocument tmp_json_document;

    int key_selector = QTime::currentTime().second();
    this->tmp_process_msg->AES_256_change_key(client_keys[key_selector].key, client_keys[key_selector].iv);

    QString tmp_string = ui->textEdit->toPlainText();
    tmp_string.replace("\n", "<br/>");
    QByteArray tmp_byteArray = tmp_string.toUtf8();
    qDebug() << tmp_byteArray << '\n';
    this->tmp_process_msg->AES_256_process(tmp_byteArray.data(), tmp_byteArray.length(), 1);

    if(!this->tmp_process_msg->ifValid()){
        ui->textBrowser->insertHtml("<b style=\"color:red\">信息加密失败</b><br>");

        ui->textBrowser->moveCursor(QTextCursor::End);

        return;
    }

    tmp_byteArray = QByteArray((const char*)this->tmp_process_msg->get_result(), this->tmp_process_msg->get_result_length());
    qDebug() << tmp_byteArray << '\n';
    qDebug() << '\n';

    tmp_json_object.insert("receiver", current_client);
    tmp_json_object.insert("sender", client_name);
    tmp_json_object.insert("type", MSG_TYPE_NORMAL);
    tmp_json_object.insert("content", QString(tmp_byteArray));
    tmp_json_object.insert("no", key_selector);

    tmp_json_document.setObject(tmp_json_object);
    QByteArray tmp_byte_array = tmp_json_document.toJson(QJsonDocument::Compact);

    bool tmp_status = send_msg(tmp_byte_array);

    QMutexLocker tmp_locker(&message_buffer_map_mtx);
    if(!tmp_status){
        message_buffer_map[current_client].push_back("<b style=\"color:red\">YOU:  sent failed</b><br>" + tmp_string + "<br>");
        ui->textBrowser->insertHtml("<b style=\"color:red\">YOU:  sent failed</b><br>" + tmp_string + "<br>");
    }else{
        message_buffer_map[current_client].push_back("<b style=\"color:red\">YOU:</b> <br>" + tmp_string + "<br>");
        ui->textBrowser->insertHtml("<b style=\"color:red\">YOU:</b><br>" + tmp_string + "<br>");
    }

    ui->textEdit->clear();
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::on_filter_button_clicked(void){
    m_proxy_model->setFilterFixedString(QString::number(1));
}

void MainWindow::on_cancel_button_clicked(void){
    m_proxy_model->setFilterFixedString(QString());
}
