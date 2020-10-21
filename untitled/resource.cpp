#include "resource.h"

QList<QString> Resource::forbid_char_in_password = {"#", "^", ",", ".", "/", " ", "(", ")", "{", "}"};

void Resource::my_memset(void *data, int size){
    char* tmp_ptr = (char*)data;
    for(int i = 0; i < size; ++i){
        *tmp_ptr = 0;
        ++tmp_ptr;
    }
}

void Resource::my_memcpy(const void *source, void *dest, int length){
    const char* tmp_ptr_src = (const char*)source;
    char* tmp_ptr_dest = (char*)dest;

    for(int i = 0; i < length; ++i){
        *tmp_ptr_dest = *tmp_ptr_src;
        ++tmp_ptr_src;
        ++tmp_ptr_dest;
    }
}
