#ifndef RESOURCE_H
#define RESOURCE_H

#include <QString>
#include <QList>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 22233

class Resource{
public:
    static QList<QString> forbid_char_in_password;
    static void my_memset(void* data, int size);
    static void my_memcpy(const void* source, void* dest, int length);
};
#endif // RESOURCE_H
