#include "mainwindow.h"
#include "ui_mainwindow.h"

QStandardItemModel* MainWindow::m_model;
QSortFilterProxyModel* MainWindow::m_proxy_model;
item_delegate* MainWindow::m_delegate;

bool MainWindow::if_continue_tag;
QMutex MainWindow::if_continue_mtx;

QMutex MainWindow::socket_mtx;
QTcpSocket* MainWindow::socket;

double MainWindow::msg_no;
QString* MainWindow::client_name;
QString MainWindow::current_client;

QMutex MainWindow::message_buffer_map_mtx;
QMap<QString, QVector<QString>> MainWindow::message_buffer_map;
QVector<QString>* MainWindow::item_message_buffer_ptr;

QByteArray MainWindow::byte_array_keep_alive;
QByteArray MainWindow::byte_array_get_user_list;

MainWindow::MainWindow(QWidget *parent, QTcpSocket* tmp_socket, QString* tmp_client_name)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = tmp_socket;
    client_name = tmp_client_name;

    if_continue_tag = true;
    msg_no = 0;

    //connect(socket, SIGNAL(disconnected()), this, SLOT(on_connection_lost()));
    connect(ui->listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(on_client_clicked(const QModelIndex&)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_send_button_clicked()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(on_filter_button_clicked()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(on_cancel_button_clicked()));


    QFontDatabase database;
    ui->textBrowser->setFont(QFont(database.families(QFontDatabase::SimplifiedChinese)[0], 16));

    init_data();

    m_delegate = new item_delegate(this);
    ui->listView->setSpacing(5);
    ui->listView->setItemDelegate(m_delegate);

    m_proxy_model->setSourceModel(m_model);
    m_proxy_model->setFilterRole(Qt::UserRole + 1);
    m_proxy_model->setDynamicSortFilter(true);

    ui->listView->setModel(m_proxy_model);
    ui->listView->setDragEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;

    delete m_model;
    delete m_delegate;
}

void MainWindow::init_data(void){
    QJsonDocument tmp_json_doc;
    QJsonObject tmp_json_obj;

    tmp_json_obj.insert("receiver", SERVER_NAME);
    tmp_json_obj.insert("sender", *client_name);
    tmp_json_obj.insert("type", MSG_TYPE_KEEPALIVE);

    tmp_json_doc.setObject(tmp_json_obj);
    byte_array_keep_alive = tmp_json_doc.toJson(QJsonDocument::Compact);

    tmp_json_obj["type"] = MSG_TYPE_GET_USER_LIST;
    tmp_json_doc.setObject(tmp_json_obj);
    byte_array_get_user_list = tmp_json_doc.toJson(QJsonDocument::Compact);

    m_model = new QStandardItemModel(this);
    m_proxy_model = new QSortFilterProxyModel(ui->listView);
    QStandardItem* tmp_item = nullptr;

    for(int i = 0; i < 20; ++i){

        tmp_item = new QStandardItem();
        tmp_item->setData(QVariant("heiheihei" + QString(i + 48)), Qt::UserRole);
        tmp_item->setData(QVariant(1), Qt::UserRole + 1);

        m_model->appendRow(tmp_item);
    }

    QVector<QString> tmp_vector;
    tmp_vector.push_back("heiheiheiheihei\n\n");
    tmp_vector.push_back("哈哈哈哈哈");
    message_buffer_map["heiheihei6"] = tmp_vector;
}

void MainWindow::keep_alive(void){
    QMutexLocker tmp_lock_socket(&socket_mtx);
    tmp_lock_socket.unlock();

    QMutexLocker tmp_lock_if_continue(&if_continue_mtx);
    tmp_lock_if_continue.unlock();

    int tmp_length = 0;

    while(1){
        tmp_lock_if_continue.relock();
        if(if_continue_tag == false){
            tmp_lock_if_continue.unlock();
            break;
        }
        tmp_lock_if_continue.unlock();

        tmp_lock_socket.relock();
        tmp_length = socket->write(byte_array_keep_alive.data(), byte_array_keep_alive.length());
        tmp_lock_socket.unlock();

        if(tmp_length <= 0){
            continue;
        }

        QThread::sleep(KEEP_ALIVE_SEND_INTERVAL_SECONDS);
    }
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
    m_model->clear();

    for(QJsonArray::iterator i = tmp_array.begin(); i != tmp_array.end(); ++i){
        tmp_item = new QStandardItem;
        tmp_item->setData(QVariant(i->toString()), Qt::UserRole);
        tmp_item->setData(QVariant(0), Qt::UserRole + 1);

        m_model->appendRow(tmp_item);
    }
}

bool MainWindow::send_msg(QByteArray &msg){
//    QMutexLocker tmp_locker(&socket_mtx);

//    int tmp_length = socket->write(msg.data(), msg.length());

//    if(tmp_length <= 0){
//        for(int i = 0; i < MSG_SEND_TRIED_TIME; ++i){
//            QThread::msleep(MSG_SEND_TRIED_INTERVAL_MSECONDS);
//            tmp_length = socket->write(msg.data(), msg.length());
//            if(tmp_length >= 0)
//                return true;
//        }
//    }else{
//        return true;
//    }

//    return false;

    return true;
}

void MainWindow::wait_message_arrival(void){
    QMutexLocker tmp_locker_socket(&socket_mtx);
    tmp_locker_socket.unlock();
    QMutexLocker tmp_locker_message_buffer(&message_buffer_map_mtx);
    tmp_locker_message_buffer.unlock();
    QMutexLocker tmp_locker_if_continue_tag(&if_continue_mtx);
    tmp_locker_if_continue_tag.unlock();

    int tmp_length = 0;
    bool tmp_status = false;
    QJsonDocument tmp_json_document;
    QJsonArray tmp_json_array;
    QString tmp_string_sender;
    QString tmp_string_content;
    QString tmp_string_type;

    QModelIndexList tmp_model_list;
    QModelIndex tmp_index;

    char data[RECEIVE_BUFFER_SIZE];
    Resource::my_memset(data, RECEIVE_BUFFER_SIZE);

    while(1){
        tmp_locker_if_continue_tag.relock();
        if(if_continue_tag == false){
            tmp_locker_if_continue_tag.unlock();
            break;
        }
        tmp_locker_if_continue_tag.unlock();

        tmp_locker_socket.relock();
        tmp_status = socket->waitForReadyRead(100);

        while(tmp_status){
            Resource::my_memset(data, RECEIVE_BUFFER_SIZE);
            tmp_length = socket->read(data, RECEIVE_BUFFER_SIZE);
            if(tmp_length > 0){
                tmp_json_document = QJsonDocument::fromRawData(data, tmp_length);
                tmp_string_sender = tmp_json_document["sender"].toString();
                tmp_string_type = tmp_json_document["type"].toString();

                if(tmp_string_type == MSG_TYPE_NORMAL){
                    tmp_string_content = tmp_json_document["content"].toString();

                    tmp_locker_message_buffer.relock();
                    message_buffer_map[tmp_string_sender].push_back(tmp_string_sender + "\n\t" + tmp_string_content + '\n');
                    tmp_locker_message_buffer.unlock();

                    tmp_model_list = m_proxy_model->match(m_proxy_model->index(0, 0), Qt::UserRole, QVariant(tmp_string_sender), 1, Qt::MatchExactly);
                    foreach(tmp_index, tmp_model_list)
                        m_proxy_model->setData(tmp_index, QVariant(1), Qt::UserRole + 1);
                }else if(tmp_string_type == MSG_TYPE_GET_USER_LIST){
                    tmp_json_array = tmp_json_document["content"].toArray();
                    refresh_user_list(tmp_json_array);
                }else if(tmp_string_type == MSG_TYPE_ERORR){
                    tmp_string_content = tmp_json_document["content"].toString();
                    QMessageBox::critical(NULL, "消息发送失败", tmp_string_content);
                }
            }

            tmp_status = socket->waitForReadyRead(100);
        }

        tmp_locker_socket.unlock();
    }
}

void MainWindow::on_client_clicked(const QModelIndex& index){
    QMutexLocker tmp_locker(&message_buffer_map_mtx);
    ui->textBrowser->clear();

    m_proxy_model->setData(index, false, Qt::UserRole + 1);
    current_client = index.data(Qt::UserRole).toString();
    item_message_buffer_ptr = &(message_buffer_map[current_client]);

    for(QVector<QString>::iterator i = item_message_buffer_ptr->begin(); i != item_message_buffer_ptr->end(); ++i){
        ui->textBrowser->insertPlainText(*i);
    }
}

void MainWindow::on_connection_lost(void){
    QMessageBox::critical(NULL, "警告", "服务器连接中断");
}

void MainWindow::on_send_button_clicked(void){
    QJsonObject tmp_json_object;
    QJsonDocument tmp_json_document;

    QString tmp_string = ui->lineEdit->text();

    tmp_json_object.insert("receiver", current_client);
    tmp_json_object.insert("sender", *client_name);
    tmp_json_object.insert("type", MSG_TYPE_NORMAL);
    tmp_json_object.insert("content", tmp_string);
    tmp_json_object.insert("no", msg_no++);

    tmp_json_document.setObject(tmp_json_object);
    QByteArray tmp_byte_array = tmp_json_document.toJson(QJsonDocument::Compact);

    bool tmp_status = send_msg(tmp_byte_array);

    QMutexLocker tmp_locker(&message_buffer_map_mtx);
    if(!tmp_status){
        message_buffer_map[current_client].push_back("YOU:\tsend failed\n\t" + tmp_string + '\n');
        ui->textBrowser->insertPlainText("YOU:\tsend failed\n\t" + tmp_string + '\n');
    }else{
        message_buffer_map[current_client].push_back("YOU:\n\t" + tmp_string + '\n');
        ui->textBrowser->insertPlainText("YOU:\n\t" + tmp_string + '\n');
    }

    ui->lineEdit->clear();
}

void MainWindow::on_filter_button_clicked(void){
    m_proxy_model->setFilterFixedString(QString::number(1));
}

void MainWindow::on_cancel_button_clicked(void){
    m_proxy_model->setFilterFixedString(QString());
}
