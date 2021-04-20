#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qrcode_win.h"

#include <QNetworkInterface>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QPainter>
#include <QBrush>
#include <QRect>
#include <QRectF>
#include "qrencode.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->listenPort = 23333;

    this->webSocketServer = new QWebSocketServer("my server", QWebSocketServer::NonSecureMode, nullptr);
    connect(this->webSocketServer, SIGNAL(newConnection()), this, SLOT(onWebSocketConnectionComing()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(onClearButtonClicked()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(onQRCodeButtonClicked()));

    this->tmp_websocket = nullptr;
    this->webSocket_pc = nullptr;
    this->webSocket_phone = nullptr;
    this->pixmap = nullptr;
    this->tmp_qrcode_win = nullptr;

    if(webSocketServer->listen(QHostAddress::Any, listenPort)){
        ui->textBrowser->append("open port successfully");
    }else{
        ui->textBrowser->append("can't open port");
    }
}

void MainWindow::onWebSocketConnectionComing(){
    this->tmp_websocket = this->webSocketServer->nextPendingConnection();
    connect(this->tmp_websocket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocketTmpMsgComing(const QString&)));
}

void MainWindow::onWebSocketTmpMsgComing(const QString &message){
    QJsonDocument tmp_json_document = QJsonDocument::fromJson(message.toUtf8());
    if(tmp_json_document["msg_type"].toInt(0) != 1){
        this->tmp_websocket->close();
        this->tmp_websocket = nullptr;
    }else{
        this->clientId = tmp_json_document["client_id"].toInt();
        if(tmp_json_document["client_type"].toInt() == 1){
            this->webSocket_pc = this->tmp_websocket;
            connect(this->webSocket_pc, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocketPCMsgComing(const QString&)));
            connect(this->webSocket_pc, SIGNAL(disconnected()), this, SLOT(onWebSocketPCDisconnected()));
            ui->textBrowser->append("new connection comming: PC");
        }else if(tmp_json_document["client_type"].toInt() == 2){
            this->webSocket_phone = this->tmp_websocket;
            connect(this->webSocket_phone, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocketPhoneMsgComing(const QString&)));
            connect(this->webSocket_phone, SIGNAL(disconnected()), this, SLOT(onWebSocketPhoneDisconnected()));
            this->webSocket_pc->sendTextMessage(message);
            ui->textBrowser->append("new connection comming: Phone");
        }
        disconnect(this->tmp_websocket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocketTmpMsgComing(const QString&)));
        this->tmp_websocket = nullptr;
    }
}



void MainWindow::onWebSocketPCMsgComing(const QString& message){
    ui->textBrowser->append("new msg from PC:");
    ui->textBrowser->append(message);

    this->webSocket_phone->sendTextMessage(message);
}

void MainWindow::onWebSocketPhoneMsgComing(const QString& message){
    ui->textBrowser->append("new msg from Phone:");
    ui->textBrowser->append(message);

    this->webSocket_pc->sendTextMessage(message);
}


void MainWindow::onWebSocketPCDisconnected(){
    ui->textBrowser->append("WebSocket PC disconnected");
    this->webSocket_pc->close();
    //delete this->webSocket_pc;
    this->webSocket_pc = nullptr;
}

void MainWindow::onWebSocketPhoneDisconnected(){
    ui->textBrowser->append("WebSocket Phone disconnected");
    this->webSocket_phone->close();
    //delete this->webSocket_phone;
    this->webSocket_phone = nullptr;
}

void MainWindow::onClearButtonClicked(){
    ui->textBrowser->clear();
}

void MainWindow::onQRCodeButtonClicked(void){

    if(this->tmp_qrcode_win == nullptr){
        tmp_qrcode_win = new QRCode_win(nullptr, this->listenPort, this->clientId);
        tmp_qrcode_win->show();
    }else{
        tmp_qrcode_win->show();
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    this->webSocketServer->close();
    delete webSocketServer;

    if(this->pixmap != nullptr){
        delete this->pixmap;
    }

    if(this->tmp_websocket != nullptr){
        this->tmp_websocket->close();
        delete this->tmp_websocket;
    }

    if(this->webSocket_pc != nullptr){
        this->webSocket_pc->close();
        delete this->webSocket_pc;
    }

    if(this->webSocket_phone != nullptr){
        this->webSocket_phone->close();
        delete this->webSocket_phone;
    }
}

