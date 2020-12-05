#ifndef RESOURCE_H
#define RESOURCE_H

#include <QString>
#include <QList>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 22233
#define SERVER_NAME "server"

#define LOGIN_PASSWD_TYPE "login_passwd"

#define KEEP_ALIVE_SEND_INTERVAL_SECONDS 4
#define MSG_SEND_TRIED_TIME 3
#define MSG_SEND_TRIED_INTERVAL_MSECONDS 500
#define RECEIVE_BUFFER_SIZE 1024

#define MSG_TYPE_ERORR "error"
#define MSG_TYPE_KEEPALIVE "keep"
#define MSG_TYPE_GET_USER_LIST "get"
#define MSG_TYPE_NORMAL "msg"
#define MSG_VALUE_FAILED "-1"

#define MESSAGE_SPLIT '\n'

#define SERVER_CERT_COMMEN_NAME "DEVILcong2"
#define SERVER_CERT_EMAIL_ADDR "liangyuecong@sina.com"

#define AES_CLIENT_KEY_NUM 30
#define AES_SERVER_KEY_NUM 6
#define AES_PROCESS_TRY_TIME 3
#define SERVER_KEYS_FILE ":/sources/server_keys"
#define CLIENT_KEYS_FILE ":/sources/client_keys"

struct aes_key_item_t{
    unsigned char key[32];
    unsigned char iv[16];
};

class Resource{
public:
    static QList<QString> forbid_char_in_password;
    static void my_memset(void* data, int size);
    static void my_memcpy(const void* source, void* dest, int length);
};
#endif // RESOURCE_H
