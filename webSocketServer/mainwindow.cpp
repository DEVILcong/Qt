#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->listenPort = 23333;

    this->webSocketServer = new QWebSocketServer("my server", QWebSocketServer::NonSecureMode, nullptr);
    connect(this->webSocketServer, SIGNAL(newConnection()), this, SLOT(onWebSocketConnectionComing()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(clearButtonClicked()));
    this->webSocket1 = nullptr;
    this->webSocket2 = nullptr;

    if(webSocketServer->listen(QHostAddress::Any, listenPort)){
        ui->textBrowser->append("open port successfully");
    }else{
        ui->textBrowser->append("can't open port");
    }
}

void MainWindow::onWebSocketConnectionComing(){
    if(this->webSocket1 == nullptr){
        this->webSocket1 = this->webSocketServer->nextPendingConnection();

        connect(this->webSocket1, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocket1MsgComing(const QString&)));
        connect(this->webSocket1, SIGNAL(disconnected()), this, SLOT(onWebSocket1Disconnected()));
        ui->textBrowser->append("new connection comming 1");
    }else{
        this->webSocket2 = this->webSocketServer->nextPendingConnection();

        connect(this->webSocket2, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocket2MsgComing(const QString&)));
        connect(this->webSocket2, SIGNAL(disconnected()), this, SLOT(onWebSocket2Disconnected()));
        ui->textBrowser->append("new connection comming 2");
    }

}

void MainWindow::onWebSocket1MsgComing(const QString& message){
    ui->textBrowser->append("new msg from 1:");
    ui->textBrowser->append(message);

    if(this->webSocket2 != nullptr){
        this->webSocket2->sendTextMessage(message);
    }
}

void MainWindow::onWebSocket2MsgComing(const QString& message){
    ui->textBrowser->append("new msg from 2:");
    ui->textBrowser->append(message);

    if(this->webSocket1 != nullptr){
        this->webSocket1->sendTextMessage(message);
    }
}


void MainWindow::onWebSocket1Disconnected(){
    ui->textBrowser->append("websocket1 disconnected");
}

void MainWindow::onWebSocket2Disconnected(){
    ui->textBrowser->append("websocket2 disconnected");
}

void MainWindow::clearButtonClicked(){
    ui->textBrowser->clear();
}

MainWindow::~MainWindow()
{
    delete ui;

    this->webSocketServer->close();
    delete webSocketServer;

    if(this->webSocket1 != nullptr){
        this->webSocket1->close();
        delete this->webSocket1;
    }

    if(this->webSocket2 != nullptr){
        this->webSocket2->close();
        delete this->webSocket2;
    }
}

