#include "blockchain.h"

#include <iostream>
#include <algorithm>
#include <chrono>

Crypto Blockchain::rsa; // static rsa member

size_t Blockchain::GetTimestamp() { // static timestamp query
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string Blockchain::GenerateNonce() { // static nonce generator
    return rsa.prng_generate();
}

void Blockchain::PrintBlock(const Block& block) { // static print block method
    std::stringstream owner, sighash, nonce;
    for(uint8_t c : rsa.sha256_hash(block.owner)) owner << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    for(uint8_t c : block.signature.hash) sighash << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    for(uint8_t c : block.nonce) nonce << std::hex << std::setw(2) << std::setfill('0') << (int)c;

    std::cout << "-----------------------------\n"
              << "Block [" << block.id << "] <- (" << (block.id == 0 ? "null/root" : std::to_string(block.previd)) << ")\n"
              << "Timestamp @ " << block.timestamp << "\n"
              << "Owner: " << owner.str() << "\n"
              << "Data: " << block.data << "\n"
              << "Signature Hash: " << sighash.str() << "\n"
              << "Nonce: " << nonce.str() << "\n";
}



Blockchain::Blockchain(): nextid(0) {

}

Blockchain::~Blockchain() {

}

void Blockchain::UpdateKeypair(const KeyPair& keypair) {
    rsa.ClearKeys();
    if(!keypair.publicKey.empty()) rsa.ImportKey(keypair.publicKey);
    if(!keypair.privateKey.empty()) rsa.ImportKey(keypair.privateKey);
}

void Blockchain::SetCurrentKeypair(const KeyPair& keypair) {
    currentUser = keypair; // updates internal keypair for current user
}

std::string Blockchain::CalculateBlockHash(const Block& block) {
    std::string rawdata;
    
    rawdata.append(reinterpret_cast<const char*>(&block.id), sizeof(block.id));
    rawdata.append(reinterpret_cast<const char*>(&block.previd), sizeof(block.previd));
    rawdata.append(reinterpret_cast<const char*>(&block.timestamp), sizeof(block.timestamp));
    rawdata += block.prevhash + block.owner + block.nonce + block.data + block.signature.hash + block.signature.signature;

    return rsa.sha256_hash(rawdata);
}

std::string Blockchain::CalculateBlockSignatureHash(const Block& block) {
    Block copy(block);
    copy.signature.hash.clear();
    copy.signature.signature.clear();

    return CalculateBlockHash(copy);
}

bool Blockchain::SignBlock(Block& block) {
    if(!block.signature.hash.empty()) return false; // block already signed

    UpdateKeypair(currentUser); // set key to current user

    Signature sig;
    sig.hash = CalculateBlockSignatureHash(block);
    sig.signature = rsa.SignHash(sig.hash);

    if(sig.hash.empty() || sig.signature.empty() || !rsa.VerifyHash(sig.signature, sig.hash)){
        return false; // failed to sign block
    }

    block.signature = sig; // copy signature onto new block
    return true;
}

bool Blockchain::ValidateBlockSignature(const Block& block) {
    rsa.ClearKeys();

    if(block.id == 0){ // validate root block
        if(rsa.sha256_hash(name) != block.prevhash) return false; // invalid root hash

        if(!rsa.ImportKey(block.owner)){ // update public key to root owner
            return false;
        }
    } else {
        Block prevBlock;
        if(!FindBlock(block.previd, prevBlock)){
            std::cout << "Previous block doesn't exist!\n";
            return false; // no previous block
        }

        if(CalculateBlockHash(prevBlock) != block.prevhash){
            return false; // broken chain
        }

        if(!rsa.ImportKey(prevBlock.owner)){ // update public key
            return false; // failed to import key
        }
    }

    if(block.signature.hash != CalculateBlockSignatureHash(block)){
        std::cout << "Signature hash mismatch!\n";
        return false; // signature hash isn't valid
    }

    return rsa.VerifyHash(block.signature.signature, block.signature.hash);
}

bool Blockchain::FindBlock(uint32_t id, Block& found) {
    auto it = std::find_if(chain.begin(), chain.end(), [&](const Block& block) -> bool {
        return block.id == id;
    });
    if(it == chain.end()) return false;

    found = *it;
    return true;
}

bool Blockchain::CreateBlock(const Block& prevBlock, const std::string& newOwner, const std::string& data) {
    std::string owner(newOwner);

    if(!ValidateBlockSignature(prevBlock)){ // cannot stem off an invalid block
        std::cout << "Stem block is invalid\n";
        return false;
    }

    UpdateKeypair(currentUser); // set key to current user

    if(owner.empty()) owner = rsa.ExportPublicKey(); // if no new owner, ownership will not change
    
    Block newBlock {}; // default construct
    newBlock.prevhash = CalculateBlockHash(prevBlock);
    newBlock.timestamp = GetTimestamp();
    newBlock.nonce = GenerateNonce();
    newBlock.id = nextid;
    newBlock.previd = prevBlock.id;
    newBlock.owner = owner;
    newBlock.data = data;

    if(!SignBlock(newBlock)){ // sign the block with my key
        std::cout << "New block failed the signature\n";
        return false;
    }

    if(!ValidateBlockSignature(newBlock)){ // validate newly signed block to make sure it was signed with a valid key
        std::cout << "New block failed to be validated. This could be because the new block was signed by the incorrect key\n";
        return false;
    }

    ++nextid;
    chain.emplace_back(newBlock);
    return true;
}

bool Blockchain::GenerateNewBlockChain(const std::string& newName) {
    if(!GenerateNewKeypair()){
        std::cout << "failed to generate keypair\n";
        return false;
    }

    chain.clear();
    nextid = 1;
    name = newName;

    Block rootBlock {}; // default construct
    rootBlock.prevhash = rsa.sha256_hash(name);
    rootBlock.timestamp = GetTimestamp();
    rootBlock.nonce = GenerateNonce();
    rootBlock.owner = currentUser.publicKey;
    rootBlock.previd = 0;
    rootBlock.id = 0;
    
    if(!SignBlock(rootBlock)) return false; // failed to sign root block

    chain.push_back(rootBlock);

    return true;
}

bool Blockchain::GenerateNewKeypair() {
    // Generate New KeyPair
    KeyPair newkeys;
    if(!rsa.GenerateKeypair()) return false; // failed to generate keypair

    // Export keys
    newkeys.publicKey = rsa.ExportPublicKey();
    newkeys.privateKey = rsa.ExportPrivateKey();

    SetCurrentKeypair(newkeys); // update blockchain default keypair for this session

    return true;
}


bool Blockchain::ExportKeys(const std::string& pubPath, const std::string& privPath) {
    auto exportKey = [](const std::string& path, const std::string& key) -> bool {
        std::ofstream file(path, std::ios::out | std::ios::binary);
        if(!file.is_open()){
            std::cout << "write file error\n";
            return false;
        }

        bool result = file.write(key.data(), key.size()).good();

        file.close();
        return result;
    };

    bool success = true;
    success &= exportKey(pubPath, currentUser.publicKey);
    if(!privPath.empty()){
        success &= exportKey(privPath, currentUser.privateKey);
    }

    return true;
}

bool Blockchain::ImportKey(const std::string& path, int type) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if(!file.is_open()){
        std::cout << "read file error\n";
        return false;
    }
    std::string key;
    while(!file.eof()){
        char chunk[64];
        std::streamsize read = file.readsome(chunk, sizeof(chunk));
        if(read) key.append(chunk, read); else break; // append cache to key
    }
    file.close();

    switch(type){
        case PK_PUBLIC:
            currentUser.publicKey = key;
        break;
        case PK_PRIVATE:
            currentUser.privateKey = key;
        break;
        default:
            return false;
    }

    return rsa.ImportKey(key);
}

bool Blockchain::ExportBlockChain(const std::string& path) {

    DataManipulator writer;
    
    FileHeader header;
    header.id = FILE_ID;
    header.version = FILE_VERSION;
    header.blockCount = GetBlockChainSize();

    writer.writeData(header);
    
    writer.writeString(name);

    for(const Block& block : chain){
        writer.writeData(block.id);
        writer.writeData(block.previd);
        writer.writeData(block.timestamp);

        writer.writeString(block.prevhash);
        writer.writeString(block.owner);
        writer.writeString(block.nonce);
        writer.writeString(block.data);

        writer.writeString(block.signature.hash);
        writer.writeString(block.signature.signature);
    }

    std::stringstream filebuffer;
    if(!writer.exportData(filebuffer)) return false;

    std::ofstream file(path, std::ios::out | std::ios::binary);
    if(!file.is_open()){
        std::cout << "write file error\n";
        return false;
    }

    bool result = (file << filebuffer.rdbuf()).good();

    file.close();
    return result;
}

bool Blockchain::ImportBlockChain(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);

    if(!file.is_open()) return false;

    std::stringstream buf;
    buf << file.rdbuf();
    file.close();
    if(!buf.good()) return false;

    const std::string& data = buf.str();

    DataManipulator reader(data.data(), data.size());
    
    FileHeader header;
    reader.readData(header);

    if(header.id != FILE_ID){
        std::cout << "invalid file\n";
        return false; // invalid file header
    }

    if(header.version > FILE_VERSION){
        std::cout << "file version unavailable\n";
        return false;
    }

    nextid = header.blockCount;
    reader.readString(name);
    std::cout << "importing \"" << name << "\" blockchain\n";
    size_t sc = 0;
    for(size_t i=0; i < header.blockCount; ++i){
        Block block {};
        bool valid = true;

        valid &= reader.readData(block.id);
        valid &= reader.readData(block.previd);
        valid &= reader.readData(block.timestamp);

        valid &= reader.readString(block.prevhash);
        valid &= reader.readString(block.owner);
        valid &= reader.readString(block.nonce);
        valid &= reader.readString(block.data);

        valid &= reader.readString(block.signature.hash);
        valid &= reader.readString(block.signature.signature);

        if(!valid){
            std::cout << "Failed to load block: End Of Stream\n";
            break;
        }

        std::cout << "Importing block [" << block.id << "] ...";
        
        if(!ValidateBlockSignature(block)){
            std::cout << " failed!                                            \n";
            continue;
        }

        chain.emplace_back(std::move(block));
        std::cout << " success                                    \r";
        ++sc;
    }

    std::cout << "\n" << sc << " blocks imported successfully!\n";

    return true;
}