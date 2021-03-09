#include "mainwindow.h"
#include "login.h"

#include <QApplication>
#include <QMessageBox>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslKey>
#include <QFile>
#include <QList>

struct aes_key_item_t server_keys[AES_SERVER_KEY_NUM];
struct aes_key_item_t client_keys[AES_CLIENT_KEY_NUM];

QJsonObject config_json;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSslSocket ssl_socket;

    ssl_socket.setPeerVerifyMode(QSslSocket::QueryPeer);

    QFile server_key_file(SERVER_KEYS_FILE);
    QFile client_key_file(CLIENT_KEYS_FILE);
    QFile config_file(CONFIG_FILE_PATH);

    QByteArray tmp_byte_array;
    QJsonDocument tmp_json_doc;

    if(!(server_key_file.open(QIODevice::ReadOnly) && client_key_file.open(QIODevice::ReadOnly))){
        QMessageBox::critical(NULL, "错误", "无法打开密钥文件");
        return 0;
    }

    int length = server_key_file.read((char*)&server_keys, AES_SERVER_KEY_NUM*sizeof(struct aes_key_item_t));
    if(length < (6 * sizeof(struct aes_key_item_t))){
        QMessageBox::critical(NULL, "错误", "读取密钥失败");
        server_key_file.close();
        client_key_file.close();
        return 0;
    }

    length = client_key_file.read((char*)&client_keys, AES_CLIENT_KEY_NUM*sizeof(struct aes_key_item_t));
    if(length < (30 * sizeof(struct aes_key_item_t))){
        QMessageBox::critical(NULL, "错误", "读取密钥失败");
        server_key_file.close();
        client_key_file.close();
        return 0;
    }

    server_key_file.close();
    client_key_file.close();

    if(!config_file.open(QIODevice::ReadOnly)){
        QMessageBox::critical(NULL, "错误", "读取配置文件失败");
        return 0;
    }

    tmp_byte_array = config_file.readAll();
    tmp_json_doc = QJsonDocument::fromJson(tmp_byte_array);
    if(tmp_json_doc.isNull()){
        QMessageBox::critical(NULL, "错误", "读取配置文件失败");
        config_file.close();
        return 0;
    }

    config_json = tmp_json_doc.object();
    config_file.close();

    MainWindow w(nullptr);
    login l(nullptr, &w, &ssl_socket);
    l.show();

    return a.exec();
}
