#include "simple_pkc.h"
#include <iostream>

Crypto::Crypto(): salt_length(8), error(false) {
    if(!InitSystem()){
        error = true;
    }
}

Crypto::~Crypto() {
    unregister_hash(&sha256_desc);
    unregister_prng(&sprng_desc);
}

bool Crypto::InitSystem() {
    if(register_prng(&sprng_desc) == -1){
        std::cout << "prng failure\n";
        return false;
    }

    if(register_prng(&yarrow_desc) == -1){
        std::cout << "yarrow failure\n";
        return false;
    }

    if(register_hash(&sha256_desc) == -1){
        std::cout << "sha256 failure\n";
        return false;
    }

    ltc_mp = ltm_desc;

    prng_idx = find_prng("sprng");
    hash_idx = find_hash("sha256");

    return true;
}

std::string Crypto::sha256_hash(const std::string& data) {
    char hashbuf[32]; // SHA256 32 bytes
    unsigned long hashlen = sizeof(hashbuf);
    int code = hash_memory(hash_idx, (const uint8_t*)data.data(), data.size(), (uint8_t*)hashbuf, &hashlen);
    if(code != CRYPT_OK){
        std::cout << "hash failure: " << error_to_string(code) << "\n";
        return "";
    }
    std::string hash(hashbuf, hashlen);
    return hash;
}

std::string Crypto::prng_generate() {
    uint8_t out[64];
    
    unsigned long read = sprng_read(out, sizeof(out), NULL);
    if(!read){
        std::cout << "sprng failed\n";
        return "";
    }
    std::string data(reinterpret_cast<const char*>(out), read);

    return data;
}

bool Crypto::GenerateKeypair(int size) {
    int code = rsa_make_key(NULL, prng_idx, size, 65537, &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return false;
    }

    return true;
}

bool Crypto::ImportKey(const std::string& key) {
    int code = rsa_import((const uint8_t*)key.data(), key.size(), &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return false;
    }

    return true;
}

void Crypto::ClearKeys() {
    memset(reinterpret_cast<char*>(&keypair), 0, sizeof(keypair)); // zero-memory key
}

std::string Crypto::ExportPublicKey() {
    std::string key;
    char out[1024 * 6];
    unsigned long len = sizeof(out);
    
    if(keypair.N == NULL) return "";

    int code = rsa_export((uint8_t*)out, &len, PK_PUBLIC, &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return "";
    }

    key.assign(out, len);
    return key;
}

std::string Crypto::ExportPrivateKey() {
    if(keypair.type != PK_PRIVATE) return "";

    std::string key;
    char out[1024 * 6];
    unsigned long len = sizeof(out);
    
    if(keypair.N == NULL) return "";

    int code = rsa_export((uint8_t*)out, &len, PK_PRIVATE, &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return "";
    }

    key.assign(out, len);
    return key;
}

std::string Crypto::EncryptKey(const std::string& key) {
    std::string output;
    char out[1024 * 6];
    unsigned long len = sizeof(out);
    int code = rsa_encrypt_key((const uint8_t*)key.data(), key.size(), (uint8_t*)out, &len, nullptr, 0, NULL, prng_idx, hash_idx, &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return "";
    }

    output.assign(out, len);
    return output;
}

std::string Crypto::DecryptKey(const std::string& enckey) {
    if(keypair.type != PK_PRIVATE) return "";

    std::string output;
    char out[1024 * 6];
    unsigned long len = sizeof(out);
    int status;
    int code = rsa_decrypt_key((const uint8_t*)enckey.data(), enckey.size(), (uint8_t*)out, &len, nullptr, 0, hash_idx, &status, &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return "";
    }
    if(status != 1){
        std::cout << "invalid status\n";
        return "";
    }
    output.assign(out, len);
    return output;
}

std::string Crypto::SignData(const std::string& data) {
    if(keypair.type != PK_PRIVATE) return "";

    std::string hash = sha256_hash(data);
    std::string output;
    char out[1024 * 2];
    unsigned long len = sizeof(out);
    int code = rsa_sign_hash((const uint8_t*)hash.data(), hash.size(), (uint8_t*)out, &len, NULL, prng_idx, hash_idx, salt_length, &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return "";
    }

    output.assign(out, len);
    return output;
}

std::string Crypto::SignHash(const std::string& hash) {
    if(keypair.type != PK_PRIVATE) return "";

    std::string output;
    char out[1024 * 2];
    unsigned long len = sizeof(out);
    int code = rsa_sign_hash((const uint8_t*)hash.data(), hash.size(), (uint8_t*)out, &len, NULL, prng_idx, hash_idx, salt_length, &keypair);
    if(code != CRYPT_OK){
        std::cout << "key failure: " << error_to_string(code) << "\n";
        return "";
    }

    output.assign(out, len);
    return output;
}

bool Crypto::VerifyData(const std::string& sighash, const std::string& data) {
    std::string hash = sha256_hash(data);

    int status;
    int code = rsa_verify_hash((const uint8_t*)sighash.data(), sighash.size(), (const uint8_t*)hash.data(), hash.size(), hash_idx, salt_length, &status, &keypair);
    if(code != CRYPT_OK) return false;

    return (status == 1);
}

bool Crypto::VerifyHash(const std::string& sighash, const std::string& hash) {

    int status;
    int code = rsa_verify_hash((const uint8_t*)sighash.data(), sighash.size(), (const uint8_t*)hash.data(), hash.size(), hash_idx, salt_length, &status, &keypair);
    if(code != CRYPT_OK) return false;

    return (status == 1);
}