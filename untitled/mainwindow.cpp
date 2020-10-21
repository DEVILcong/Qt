#include "mainwindow.h"
#include "ui_mainwindow.h"

QMutex MainWindow::message_buffer_map_mtx;
QMap<QString, message_buffer_t> MainWindow::message_buffer_map;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
        tmp_item->setData(QVariant("heiheihei"), Qt::UserRole);
        tmp_item->setData(QVariant(true), Qt::UserRole + 1);

        m_model->appendRow(tmp_item);
    }
}
