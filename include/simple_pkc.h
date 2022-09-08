#pragma once

#define LTM_DESC
#include "tomcrypt.h"

#include <string>
#include <algorithm>

class Crypto {
    rsa_key keypair;
    int prng_idx, hash_idx, salt_length;
    bool error;

    bool InitSystem();

public:
    Crypto();
    virtual ~Crypto();

    std::string sha256_hash(const std::string& data);
    std::string prng_generate();


    bool GenerateKeypair(int size=256);
    bool ImportKey(const std::string& key);
    void ClearKeys();

    std::string ExportPublicKey();
    std::string ExportPrivateKey();

    std::string EncryptKey(const std::string& key);
    std::string DecryptKey(const std::string& enckey);

    std::string SignData(const std::string& data);
    std::string SignHash(const std::string& hash);
    bool VerifyData(const std::string& sighash, const std::string& data);
    bool VerifyHash(const std::string& sighash, const std::string& hash);

    inline void SaltLength(int length) { salt_length = length; }
    inline int SaltLength() const { return salt_length; }

};