#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QMap>
#include <QFontDatabase>
#include <QDebug>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QThread>

#include "item_delegate.h"
#include "resource.h"

struct message_buffer_t{
    QMutex mtx;
    QString data;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, QTcpSocket* tmp_socket = nullptr, QString* tmp_client_name = nullptr);
    ~MainWindow();

public slots:
    void on_connection_lost(void);
    void on_client_clicked(const QModelIndex& index);
    void on_send_button_clicked(void);
    void on_filter_button_clicked(void);
    void on_cancel_button_clicked(void);

private:
    Ui::MainWindow *ui;
    static QStandardItemModel* m_model;
    static QSortFilterProxyModel* m_proxy_model;
    static item_delegate* m_delegate;

    static bool if_continue_tag;
    static QMutex if_continue_mtx;

    static QMutex socket_mtx;
    static QTcpSocket* socket;
    static QString* client_name;

    static QString current_client;
    static double msg_no;

    static QMutex message_buffer_map_mtx;
    static QMap<QString, QVector<QString>> message_buffer_map;
    static QVector<QString>* item_message_buffer_ptr;

    static QByteArray byte_array_keep_alive;
    static QByteArray byte_array_get_user_list;

    void init_data(void);
    static void keep_alive(void);
    static bool get_user_list(void);
    static void refresh_user_list(QJsonArray& tmp_array);
    static bool send_msg(QByteArray& msg);
    static void wait_message_arrival(void);
};
#endif // MAINWINDOW_H
