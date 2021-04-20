#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QPixmap>
#include "qrcode_win.h"

#define MSG_TYPE_HEADER "msg_type"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onWebSocketConnectionComing(void);
    void onWebSocketTmpMsgComing(const QString& message);
    void onWebSocketPCMsgComing(const QString& message);
    void onWebSocketPhoneMsgComing(const QString& message);
    void onWebSocketPCDisconnected(void);
    void onWebSocketPhoneDisconnected(void);

    void onClearButtonClicked(void);
    void onQRCodeButtonClicked(void);

private:
    int listenPort;
    int clientId;

    QPixmap *pixmap;
    QRCode_win *tmp_qrcode_win;
    QWebSocketServer* webSocketServer;
    QWebSocket *webSocket_pc, *webSocket_phone, *tmp_websocket;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
