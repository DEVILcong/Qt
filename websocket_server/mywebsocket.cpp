#include "mywebsocket.h"
#include <QMap>
#include <QFile>
#include <QObject>
#include <QJsonDocument>
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
       this->success_tag = -2;
       return;
   }

   connect(this->websocket_server, SIGNAL(newConnection()), this, SLOT(onListenSocketNewConnection()));

   this->tmp_qtimer = new QTimer();
   connect(this->tmp_qtimer, SIGNAL(timeout()), this, SLOT(onTimerCleanTmpWebSockets()));
   this->tmp_qtimer->start(TMP_WEBSOCKET_CLEAN_TIMER_MS);
}
qint8 MyWebsocket::get_success_tag(){
    return this->success_tag;
}

bool MyWebsocket::read_config_file(){
    QFile tmp_file(CONFIG_FILE_PATH);
    if(!tmp_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return false;
    }

    QJsonDocument tmp_document = QJsonDocument::fromJson(tmp_file.readAll());
    if(tmp_document.isNull()){
        return false;
    }

    this->server_name = tmp_document["server name"].toString();
    this->port = tmp_document["port"].toInt();

    tmp_file.close();
    return true;
}

void MyWebsocket::onListenSocketNewConnection(void){
    static struct tmp_websocket_item *tmp_websocket_item_ptr = nullptr;

    if(this->websocket_server->hasPendingConnections()){
        QWebSocket *tmp_websocket = this->websocket_server->nextPendingConnection();
        if(tmp_websocket_map.size() > TMP_WEBSOCKET_MAX_NUM){
            tmp_websocket->close(QWebSocketProtocol::CloseCodeBadOperation,
                                 TMP_WEBSOCKET_MAX_NUM_ERROR_REASON);
            return;
        }
        tmp_websocket_item_ptr = new struct tmp_websocket_item;
        tmp_websocket_item_ptr->tmp_websocket = tmp_websocket;
        tmp_websocket_item_ptr->count_down = TMP_WEBSOCKET_ITEM_COUNT_DOWN;

        connect(tmp_websocket, SIGNAL(textMessageReceived(QString& string)), this, SLOT(onWebSocketMsgArrived(QString& string)));
        tmp_websocket_map.insert(tmp_websocket, tmp_websocket_item_ptr);
    }
}

void MyWebsocket::onTimerCleanTmpWebSockets(void){
    static QMap<QWebSocket*, struct tmp_websocket_item*>::iterator tmp_iterator0;
    static bool if_tmp_iterator0_valid = false;

    for(QMap<QWebSocket*, struct tmp_websocket_item*>::iterator tmp_iterator = tmp_websocket_map.begin();
        tmp_iterator != tmp_websocket_map.end(); ++tmp_iterator){
        if(if_tmp_iterator0_valid){
            tmp_iterator = tmp_iterator0;
            if_tmp_iterator0_valid = false;
        }

        --tmp_iterator.value()->count_down;
        if(tmp_iterator.value()->count_down < 0){
            if(tmp_iterator.value()->tmp_websocket->state() == QAbstractSocket::ConnectedState)
                tmp_iterator.value()->tmp_websocket->close(QWebSocketProtocol::CloseCodeNormal,
                                                       TMP_WEBSOCKET_HANDSHAKE_TIMEOUT);
            delete tmp_iterator.value();
            if_tmp_iterator0_valid = true;
            tmp_iterator0 = tmp_iterator = tmp_websocket_map.erase(tmp_iterator);
        }
    }
}

void MyWebsocket::onWebSocketMsgArrived(QString &string){
    static QJsonDocument tmp_document = QJsonDocument::fromJson(string.toUtf8());
    static qint8 msg_type = tmp_document[MSG_TYPE_HEADER].toInt();
    static double client_ID = tmp_document[MSG_CLIENT_ID].toDouble();
    static qint8 client_type = tmp_document[MSG_CLIENT_TYPE].toInt();
    static QWebSocket *tmp_websocket = nullptr;

    if(msg_type == MSG_TYPE_NORMAL){
        if(websocket_map.contains(client_ID) &&
                websocket_map[client_ID]->pc_websocket->state() == QAbstractSocket::ConnectedState &&
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
            QJsonObject tmp_json_object;
            tmp_json_object.insert(MSG_TYPE_HEADER, QJsonValue(MSG_TYPE_HANDSHAKE));
            tmp_websocket->sendTextMessage(QJsonDocument(tmp_json_object).toJson());
        }else{
            websocket_map[client_ID]->phone_websocket = tmp_websocket;
        }
    }
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
}
