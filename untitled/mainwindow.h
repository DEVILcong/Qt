#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMutex>
#include <QString>
#include <QMap>

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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QStandardItemModel* m_model;
    item_delegate* m_delegate;

    static QMutex message_buffer_map_mtx;
    static QMap<QString, message_buffer_t> message_buffer_map;

    void init_data(void);
};
#endif // MAINWINDOW_H
