#ifndef _PROCESS_MSG_HPP_
#define _PROCESS_MSG_HPP_

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/buffer.h>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <memory>

//#define _OUTPUT_

#define DIGEST_SIZE 32    //sha3-256
#define DIGEST_METHOD EVP_sha3_256()

#define EVP_CIPHER EVP_aes_256_cbc()
#define AES_256_KEY_LEN 32
#define AES_256_IV_LEN 16
#define AES_256_BLOCK_LEN 16

class deleter{
public:
    deleter(void);

    template<class T>
    void operator()(T* ptr);
};

class ProcessMsg{
public:
    ProcessMsg(int sha3_256__base64);
    ProcessMsg(EVP_PKEY* RSA_key);
    ProcessMsg(unsigned char* key, unsigned char* iv);
    ~ProcessMsg();

    bool ifValid(void);
    void base64_encode(const char* str, size_t length);
    void base64_decode(const char* str, size_t length);

    void digest(const char* str, size_t length);

    void RSA_encrypt(const char* str, size_t length);
    void RSA_decrypt(const char* str, size_t length);

    void AES_256_change_key(unsigned char* key, unsigned char* iv);
    void AES_256_process(const char* str, size_t length, int enc);

    unsigned char* get_result(void);
    size_t get_result_length(void);

private:
    bool isValid;
    deleter d;
    std::unique_ptr<unsigned char, deleter> buffer;
    size_t buffer_length;
    BIO* bio_base64_encode;
    BIO* bio_base64_decode;
    EVP_PKEY_CTX* evp_pkey_ctx;
    EVP_CIPHER_CTX* evp_cipher_ctx;
    
    unsigned char* aes_key;
    unsigned char* aes_iv;

    unsigned char* aes_key_tmp;
    unsigned char* aes_iv_tmp;

    void init(void);
    void print_hex(unsigned char* str, unsigned int length);

};



#endif 
