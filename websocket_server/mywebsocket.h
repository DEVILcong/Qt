#ifndef MYWEBSOCKET_H
#define MYWEBSOCKET_H

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QMap>
#include <QMutex>
#include <QTimer>

#define CONFIG_FILE_PATH ":/config/config.json"
#define WEBSOCKET_CLIENT_TYPE_PC 1
#define WEBSOCKET_CLIENT_TYPE_PHONE 2
#define TMP_WEBSOCKET_MAX_NUM 10
#define TMP_WEBSOCKET_MAX_NUM_ERROR_REASON "server has too much pending sockets"
#define TMP_WEBSOCKET_HANDSHAKE_TIMEOUT "handshake timeout"
#define TMP_WEBSOCKET_ITEM_COUNT_DOWN 6
#define TMP_WEBSOCKET_CLEAN_TIMER_MS 10000

#define MSG_TYPE_HEADER "msg_type"
#define MSG_TYPE_HANDSHAKE 1
#define MSG_TYPE_NORMAL 2
#define MSG_TYPE_ERROR 3

#define MSG_ERR_USER_UNREACHABLE "another user unreachable"

#define MSG_CLIENT_ID "client_id"
#define MSG_CLIENT_TYPE "client_type"
#define MSG_CLIENT_CONTENT "content"
#define MSG_CLIENT_CONTENT_TYPE "content_type"
#define MSG_CLIENT_EXTRA "extra"

struct websocket_pair{
    QWebSocket *pc_websocket;
    QWebSocket *phone_websocket;
    qint8 num;
};

struct tmp_websocket_item{
    QWebSocket *tmp_websocket;
    qint8 count_down;
};

class MyWebsocket : QObject
{
    Q_OBJECT
public:
    MyWebsocket();
    ~MyWebsocket();
    qint8 get_success_tag();

public slots:
    void onListenSocketNewConnection(void);
    void onTimerCleanTmpWebSockets(void);
    void onWebSocketMsgArrived(const QString &message);
    void onWebSocketDisconnect();

private:
    qint8 success_tag = 0;

    bool read_config_file();

    QString server_name;
    qint32 port;
    QTimer* tmp_qtimer;

    QMutex to_clean_websockets_mtx;
    QList<QWebSocket*> to_clean_websockets;

    QWebSocketServer* websocket_server;
    QMap<double, struct websocket_pair*> websocket_map;
    QMap<QWebSocket*, double> reverse_websocket_map;
    QMap<QWebSocket*, struct tmp_websocket_item*> tmp_websocket_map;
};

#endif // MYWEBSOCKET_H
