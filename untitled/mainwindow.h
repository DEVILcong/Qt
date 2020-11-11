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
#include <QCloseEvent>
#include <QTimer>
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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init_data(QTcpSocket* tmp_socket, QString& tmp_client_name);

public slots:
    void on_connection_lost(void);
    void on_client_clicked(const QModelIndex& index);
    void on_send_button_clicked(void);
    void on_filter_button_clicked(void);
    void on_cancel_button_clicked(void);

    void keep_alive(void);
    void on_message_arrival(void);

protected:
    void closeEvent(QCloseEvent* event);

private:
    Ui::MainWindow *ui;
    QStandardItemModel* m_model;
    QSortFilterProxyModel* m_proxy_model;
    item_delegate* m_delegate;

    bool if_continue_tag;
    QMutex if_continue_mtx;

    QMutex socket_mtx;

    QString current_client;
    double msg_no;

    QMutex message_buffer_map_mtx;
    QMap<QString, QVector<QString>> message_buffer_map;
    QVector<QString>* item_message_buffer_ptr;

    QByteArray byte_array_keep_alive;
    QByteArray byte_array_get_user_list;

    QTimer* timer;
    QTcpSocket* socket;
    QString client_name;

    bool get_user_list(void);
    void refresh_user_list(QJsonArray& tmp_array);
    bool send_msg(QByteArray& msg);
};
#endif // MAINWINDOW_H
