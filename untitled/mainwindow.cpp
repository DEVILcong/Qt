#include "mainwindow.h"
#include "ui_mainwindow.h"

QStandardItemModel* MainWindow::m_model;
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

    connect(ui->listView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(on_client_clicked(const QModelIndex&)));

    QFontDatabase database;
    ui->textBrowser->setFont(QFont(database.families(QFontDatabase::SimplifiedChinese)[0], 16));

    init_data();

    m_delegate = new item_delegate(this);
    ui->listView->setSpacing(5);
    ui->listView->setItemDelegate(m_delegate);
    ui->listView->setModel(m_model);
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
//    QStandardItem* tmp_item = nullptr;

//    for(int i = 0; i < 20; ++i){

//        tmp_item = new QStandardItem();
//        tmp_item->setData(QVariant("heiheihei" + QString(i + 48)), Qt::UserRole);
//        tmp_item->setData(QVariant(true), Qt::UserRole + 1);

//        m_model->appendRow(tmp_item);
//    }

//    QVector<QString> tmp_vector;
//    tmp_vector.push_back("heiheiheiheihei\n\n");
//    tmp_vector.push_back("哈哈哈哈哈");
//    message_buffer_map["heiheihei6"] = tmp_vector;
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
        tmp_item->setData(QVariant(false), Qt::UserRole + 1);

        m_model->appendRow(tmp_item);
    }
}

void MainWindow::on_client_clicked(const QModelIndex& index){
    QMutexLocker tmp_locker(&message_buffer_map_mtx);
    ui->textBrowser->clear();

    m_model->setData(index, false, Qt::UserRole + 1);
    current_client = index.data(Qt::UserRole).toString();
    item_message_buffer_ptr = &(message_buffer_map[current_client]);

    for(QVector<QString>::iterator i = item_message_buffer_ptr->begin(); i != item_message_buffer_ptr->end(); ++i){
        ui->textBrowser->insertPlainText(*i);
    }
}
