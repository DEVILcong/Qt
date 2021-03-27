#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocketServer>
#include <QWebSocket>

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
    void onWebSocket1MsgComing(const QString& message);
    void onWebSocket2MsgComing(const QString& message);
    void onWebSocket1Disconnected(void);
    void onWebSocket2Disconnected(void);

    void clearButtonClicked(void);

private:
    int listenPort;

    QWebSocketServer* webSocketServer;
    QWebSocket *webSocket1, *webSocket2;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
