#include "mywebsocket.h"
#include <QMap>
#include <QFile>
#include <QObject>
#include <QJsonDocument>
#include <iostream>
#include <QJsonObject>
#include <QtWebSockets/QWebSocketServer>

MyWebsocket::MyWebsocket()
{
   if(!this->read_config_file()){
       this->success_tag = -1;
       return;
   }
   websocket_server = new QWebSocketServer(this->server_name, QWebSocketServer::NonSecureMode);
   if(!websocket_server->listen(QHostAddress::Any, this->port)){
       std::cout << "ERROR: fail to open listen port" << std::endl;
       this->success_tag = -2;
       return;
   }

   connect(this->websocket_server, SIGNAL(newConnection()), this, SLOT(onListenSocketNewConnection()));

   this->tmp_qtimer = new QTimer();
   connect(this->tmp_qtimer, SIGNAL(timeout()), this, SLOT(onTimerCleanTmpWebSockets()));
   this->tmp_qtimer->start(TMP_WEBSOCKET_CLEAN_TIMER_MS);

   std::cout << "INFO: Server srart running..." << std::endl;
}
qint8 MyWebsocket::get_success_tag(){
    return this->success_tag;
}

bool MyWebsocket::read_config_file(){
    QFile tmp_file(CONFIG_FILE_PATH);
    if(!tmp_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        std::cout << "ERROR: failed to open config file" << std::endl;
        return false;
    }

    QJsonDocument tmp_document = QJsonDocument::fromJson(tmp_file.readAll());
    if(tmp_document.isNull()){
        std::cout << "ERROR: config file has syntax error" << std::endl;
        return false;
    }

    this->server_name = tmp_document["server name"].toString();
    this->port = tmp_document["port"].toInt();

    tmp_file.close();
    return true;
}

void MyWebsocket::onListenSocketNewConnection(void){
    static struct tmp_websocket_item *tmp_websocket_item_ptr = nullptr;

    while(this->websocket_server->hasPendingConnections()){
        QWebSocket *tmp_websocket = this->websocket_server->nextPendingConnection();
        if(tmp_websocket_map.size() > TMP_WEBSOCKET_MAX_NUM){
            tmp_websocket->close(QWebSocketProtocol::CloseCodeBadOperation,
                                 TMP_WEBSOCKET_MAX_NUM_ERROR_REASON);
            return;
        }
        tmp_websocket_item_ptr = new struct tmp_websocket_item;
        tmp_websocket_item_ptr->tmp_websocket = tmp_websocket;
        tmp_websocket_item_ptr->count_down = TMP_WEBSOCKET_ITEM_COUNT_DOWN;

        std::cout << "INFO: new connection comming " << tmp_websocket->peerAddress().toString().toStdString() << std::endl;

        connect(tmp_websocket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocketMsgArrived(const QString&)));
        tmp_websocket_map.insert(tmp_websocket, tmp_websocket_item_ptr);
    }
}

void MyWebsocket::onTimerCleanTmpWebSockets(void){
    for(QMap<QWebSocket*, struct tmp_websocket_item*>::iterator tmp_iterator = tmp_websocket_map.begin();
        tmp_iterator != tmp_websocket_map.end(); ++tmp_iterator){
        --tmp_iterator.value()->count_down;
        if(tmp_iterator.value()->count_down < 0){
            if(tmp_iterator.value()->tmp_websocket->state() == QAbstractSocket::ConnectedState){
                to_clean_websockets.append(tmp_iterator.value()->tmp_websocket);
            }
        }
    }

    for(QList<QWebSocket*>::iterator tmp_iterator = to_clean_websockets.begin(); tmp_iterator != to_clean_websockets.end(); ++tmp_iterator){
        std::cout << "WARNING: websocket " <<
                     (*tmp_iterator)->peerAddress().toString().toStdString() <<
                     " closed by server due to timeout" << std::endl;

        (*tmp_iterator)->close(QWebSocketProtocol::CloseCodeNormal,
                                               TMP_WEBSOCKET_HANDSHAKE_TIMEOUT);

        QMap<QWebSocket*, struct tmp_websocket_item*>::iterator tmp_iterator2 = tmp_websocket_map.find(*tmp_iterator);
        if(tmp_iterator2 != tmp_websocket_map.end()){
            delete tmp_iterator2.value();
            tmp_websocket_map.erase(tmp_iterator2);
        }
    }

    to_clean_websockets.clear();
}

void MyWebsocket::onWebSocketMsgArrived(const QString &string){
    static QJsonDocument tmp_document;
    static qint8 msg_type;
    static double client_ID;
    static qint8 client_type;
    static QWebSocket *tmp_websocket;

    tmp_document = QJsonDocument::fromJson(string.toUtf8());
    msg_type = tmp_document[MSG_TYPE_HEADER].toInt();
    client_ID = tmp_document[MSG_CLIENT_ID].toDouble();
    client_type = tmp_document[MSG_CLIENT_TYPE].toInt();
    tmp_websocket = nullptr;

//    std::cout << "INFO: recv msg " << string.toStdString() << std::endl;

    if(msg_type == MSG_TYPE_NORMAL){
//        if(!websocket_map.contains(client_ID)){
//            std::cout << "ERROR: 1" << std::endl;
//            return;
//        }

//        if(!(websocket_map[client_ID]->pc_websocket != nullptr)){
//            std::cout << "ERROR: 2" << std::endl;
//            return;
//        }

//        if(!(websocket_map[client_ID]->pc_websocket->state() == QAbstractSocket::ConnectedState)){
//            std::cout << "ERROR: 3 " << websocket_map[client_ID]->pc_websocket->state() << std::endl;
//            return;
//        }

//        if(!(websocket_map[client_ID]->phone_websocket != nullptr)){
//            std::cout << "ERROR: 4" << std::endl;
//            return;
//        }

//        if(!(websocket_map[client_ID]->phone_websocket->state() == QAbstractSocket::ConnectedState)){
//            std::cout << "ERROR: 5" << websocket_map[client_ID]->phone_websocket->state() << std::endl;
//            return;
//        }

        if(websocket_map.contains(client_ID) &&
                websocket_map[client_ID]->pc_websocket != nullptr &&
                websocket_map[client_ID]->pc_websocket->state() == QAbstractSocket::ConnectedState &&
                websocket_map[client_ID]->phone_websocket != nullptr &&
                websocket_map[client_ID]->phone_websocket->state() == QAbstractSocket::ConnectedState){
            if(client_type == WEBSOCKET_CLIENT_TYPE_PC){
                tmp_websocket = websocket_map[client_ID]->phone_websocket;
            }else if(client_type == WEBSOCKET_CLIENT_TYPE_PHONE){
                tmp_websocket = websocket_map[client_ID]->pc_websocket;
            }

            tmp_websocket->sendTextMessage(string);
        }else{
            QJsonObject tmp_json_object;
            tmp_json_object.insert(MSG_TYPE_HEADER, QJsonValue(MSG_TYPE_ERROR));
            tmp_json_object.insert(MSG_CLIENT_CONTENT, QJsonValue(string));
            tmp_json_object.insert(MSG_CLIENT_EXTRA, QJsonValue(MSG_ERR_USER_UNREACHABLE));

            std::cout << "WARNING: invalid message from " <<
                         qobject_cast<QWebSocket *>(sender())->peerAddress().toString().toStdString() <<
                         "  " << string.toStdString() << std::endl;

            qobject_cast<QWebSocket *>(sender())->sendTextMessage(
                        QJsonDocument(tmp_json_object).toJson());
        }
    }else if(msg_type == MSG_TYPE_HANDSHAKE){
        tmp_websocket = qobject_cast<QWebSocket *>(sender());

        QMap<QWebSocket *, struct tmp_websocket_item*>::iterator tmp_iterator =
                tmp_websocket_map.find(tmp_websocket);
        delete tmp_iterator.value();
        tmp_websocket_map.erase(tmp_iterator);

        if(!websocket_map.contains(client_ID)){
            struct websocket_pair* tmp_websocket_pair_ptr = new struct websocket_pair;
            tmp_websocket_pair_ptr->num = 0;
            tmp_websocket_pair_ptr->pc_websocket = nullptr;
            tmp_websocket_pair_ptr->phone_websocket = nullptr;
            websocket_map.insert(client_ID, tmp_websocket_pair_ptr);
        }
        if(client_type == WEBSOCKET_CLIENT_TYPE_PC){
            websocket_map[client_ID]->pc_websocket = tmp_websocket;
            reverse_websocket_map[websocket_map[client_ID]->pc_websocket] = client_ID;
            websocket_map[client_ID]->num += 1;
            connect(websocket_map[client_ID]->pc_websocket, SIGNAL(disconnected()), this, SLOT(onWebSocketDisconnect()));
            std::cout << "INFO: user from " <<
                tmp_websocket->peerAddress().toString().toStdString() <<
                " logged in as PC " << client_ID << std::endl;
        }else{
            websocket_map[client_ID]->phone_websocket = tmp_websocket;
            reverse_websocket_map[websocket_map[client_ID]->phone_websocket] = client_ID;
            websocket_map[client_ID]->num += 1;
            connect(websocket_map[client_ID]->phone_websocket, SIGNAL(disconnected()), this, SLOT(onWebSocketDisconnect()));
            QJsonObject tmp_json_object;
            tmp_json_object.insert(MSG_TYPE_HEADER, QJsonValue(MSG_TYPE_HANDSHAKE));
            websocket_map[client_ID]->pc_websocket->sendTextMessage(QJsonDocument(tmp_json_object).toJson());
            std::cout << "INFO: user from " <<
                tmp_websocket->peerAddress().toString().toStdString() <<
                " loged in as Phone " << client_ID << std::endl;
        }
    }
}

void MyWebsocket::onWebSocketDisconnect(){
    QWebSocket* tmp_websocket = qobject_cast<QWebSocket*>(sender());
    std::cout << "INFO: websocket from " << tmp_websocket->peerAddress().toString().toStdString() <<
         " closed" << std::endl;
    if(!reverse_websocket_map.contains(tmp_websocket)){
        std::cout << "WARNING: invalid websocket close signal from " <<
             tmp_websocket->peerAddress().toString().toStdString() << std::endl;
        return;
    }

    double tmp_client_id = reverse_websocket_map[tmp_websocket];
    websocket_map[tmp_client_id]->num -= 1;

    if(websocket_map[tmp_client_id]->num == 0){
        websocket_map.erase(websocket_map.find(tmp_client_id));
    }

    disconnect(tmp_websocket, SIGNAL(disconnected()), this, SLOT(onWebSocketDisconnect()));
    disconnect(tmp_websocket, SIGNAL(textMessageReceived(const QString&)), this, SLOT(onWebSocketMsgArrived(const QString&)));
}

MyWebsocket::~MyWebsocket(){
    if(this->websocket_server != nullptr){
        this->websocket_server->close();
        delete this->websocket_server;
    }

    if(this->tmp_qtimer != nullptr){
        delete this->tmp_qtimer;
    }

    for(QMap<double, struct websocket_pair*>::iterator tmp_iterator = websocket_map.begin();
        tmp_iterator != websocket_map.end(); ++tmp_iterator){
        if(tmp_iterator.value()->pc_websocket != nullptr){
            if(tmp_iterator.value()->pc_websocket->state() == QAbstractSocket::ConnectedState)
                tmp_iterator.value()->pc_websocket->close();
        }

        if(tmp_iterator.value()->phone_websocket != nullptr){
           if(tmp_iterator.value()->phone_websocket->state() == QAbstractSocket::ConnectedState)
                tmp_iterator.value()->phone_websocket->close();
        }

        delete tmp_iterator.value();
    }

    for(QMap<QWebSocket*, struct tmp_websocket_item*>::iterator tmp_iterator = tmp_websocket_map.begin();
        tmp_iterator != tmp_websocket_map.end(); ++tmp_iterator){
        tmp_iterator.value()->tmp_websocket->close();
        delete tmp_iterator.value();
    }

    std::cout << "INFO: Server stopped :)" << std::endl;
}
