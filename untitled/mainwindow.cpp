#include "mainwindow.h"
#include "ui_mainwindow.h"

QTcpSocket* MainWindow::socket;
QString* MainWindow::client_name;
QString MainWindow::current_client;
QMutex MainWindow::message_buffer_map_mtx;
QMap<QString, QVector<QString>> MainWindow::message_buffer_map;
QVector<QString>* MainWindow::item_message_buffer_ptr;

MainWindow::MainWindow(QWidget *parent, QTcpSocket* tmp_socket, QString* tmp_client_name)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = tmp_socket;
    client_name = tmp_client_name;

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
    m_model = new QStandardItemModel(this);
    QStandardItem* tmp_item = nullptr;

    for(int i = 0; i < 20; ++i){

        tmp_item = new QStandardItem();
        tmp_item->setData(QVariant("heiheihei" + QString(i + 48)), Qt::UserRole);
        tmp_item->setData(QVariant(true), Qt::UserRole + 1);

        m_model->appendRow(tmp_item);
    }

    QVector<QString> tmp_vector;
    tmp_vector.push_back("heiheiheiheihei\n\n");
    tmp_vector.push_back("哈哈哈哈哈");
    message_buffer_map["heiheihei6"] = tmp_vector;
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
