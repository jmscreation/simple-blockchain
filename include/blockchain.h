#pragma once

#include "simple_pkc.h"
#include "fileio.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

#define FILE_ID         3489030000
#define FILE_VERSION    100

struct FileHeader {
    size_t id, version, blockCount;
};

struct Signature {
    std::string hash, signature;
};

struct Block {
    std::string prevhash; // hash of previous block + signature
    uint32_t id, previd; // id of block and previous block
    uint64_t timestamp; // timestamp on creation
    std::string nonce; // nonce

    std::string owner; // public key of block owner
    std::string data; // signed data

    Signature signature;
};


struct KeyPair {
    std::string publicKey, privateKey;
};

class Blockchain {
    static Crypto rsa; // static RSA Crypto controller

    uint32_t nextid; // next global id
    std::vector<Block> chain; // database of blocks
    std::string name; // name of blockchain

    KeyPair currentUser; // locally stored keys for current user
public:
    static size_t GetTimestamp();
    static void PrintBlock(const Block& block);
    static std::string GenerateNonce();

    Blockchain();
    virtual ~Blockchain();

    void UpdateKeypair(const KeyPair& keypair);
    void SetCurrentKeypair(const KeyPair& keypair);

    std::string CalculateBlockHash(const Block& block);
    std::string CalculateBlockSignatureHash(const Block& block);

    bool CreateBlock(const Block& prevBlock, const std::string& newOwner, const std::string& data);
    bool SignBlock(Block& block);
    bool ValidateBlockSignature(const Block& block);

    bool ExportBlockChain(const std::string& path);
    bool ImportBlockChain(const std::string& path);
    bool GenerateNewBlockChain(const std::string& newName);
    bool GenerateNewKeypair();
    
    bool ExportKeys(const std::string& pubPath, const std::string& privPath="");
    bool ImportKey(const std::string& path, int type);

    bool FindBlock(uint32_t id, Block& found);
    inline size_t GetBlockChainSize() const { return chain.size(); }
    inline const std::vector<Block>& GetBlockChain() const { return chain; }
};