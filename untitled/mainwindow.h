#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QMap>
#include <QFontDatabase>
#include <QDebug>
#include <QTcpSocket>

#include "item_delegate.h"

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
    void on_client_clicked(const QModelIndex& index);

private:
    Ui::MainWindow *ui;
    QStandardItemModel* m_model;
    item_delegate* m_delegate;

    static QTcpSocket* socket;
    static QString* client_name;

    static QString current_client;
    static QMutex message_buffer_map_mtx;
    static QMap<QString, QVector<QString>> message_buffer_map;
    static QVector<QString>* item_message_buffer_ptr;

    void init_data(void);
};
#endif // MAINWINDOW_H
