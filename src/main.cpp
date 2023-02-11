#include "blockchain.h"
#include <iostream>

#ifdef _WIN32
#include "windows.h"
#endif
#ifdef __linux__
#endif

void PrintData(const std::string& title, const std::string& data) {
    std::cout << title << " (" << data.size() << "): ";
    for(char c : data){
        std::cout << (c == '\n' || c == '\r' ? ' ' : c);
    }
    std::cout << "\n";
}

std::vector<std::string> args;
bool FindArg(const std::string& arg){
    return (std::find(args.begin(), args.end(), arg) != args.end());
}
bool FindParam(const std::string& arg, std::string& param, int ind=1){
    
    auto it = std::find(args.begin(), args.end(), arg);
    if(it == args.end()) return false;
    for(int i=0; i < ind; i++) if(++it == args.end()) return false;
    param = *it;

    it->clear(); // consume parameter
    return true;
}
#ifdef _WIN32
bool Confirm() {
    bool yes = false, no = false;

    std::cout << "Press Y or N\n";
    do {
        yes = (GetAsyncKeyState('Y') & 0x8000);
        no = (GetAsyncKeyState('N') & 0x8000);
    } while(!yes && !no);

    return yes;
}
#endif

#ifdef __linux__

uint8_t GetKeyState()
{
    uint8_t key;
    std::cin >> key;

    if(key >= 97 || key <= 122){ // lc letter input
        key -= 32; // make upper case
    }

    return key;
}

bool Confirm() {
    bool yes = false, no = false;
    uint8_t key;
    std::cout << "Press Y or N\n";
    do {
        key = GetKeyState();
    } while(key != 'Y' && key != 'N');

    return key == 'Y';
}
#endif

bool ToInteger(const std::string& value, int64_t& out) {
    try {
        out = stoll(value);
    } catch (std::exception e) {
        std::cout << e.what() << "\n";
        return false;
    }
    return true;
}

std::string LoadFileData(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if(!file.is_open()){
        std::cout << "read file error\n";
        return "";
    }
    std::string data;
    while(!file.eof()){
        char chunk[64];
        std::streamsize read = file.readsome(chunk, sizeof(chunk));
        if(read) data.append(chunk, read); else break; // append cache to key
    }
    file.close();

    return data;
}

int main(int argc, const char** argv) {
    for(int i=1; i < argc; ++i) args.push_back(argv[i]);

    std::cout << "---------------------------------------------\n";
    
    std::string database = "BlockO.chain";
    std::string privatekey = "BlockO.key";
    std::string publickey = "BlockO.pub";

    Blockchain BlockO;

    do {

        { // generate a new blockchain
            std::string newName;
            if(FindParam("newchain", newName)){
                std::cout << "This will generate a new blockchain and overwrite the old blockchain.\nContinue?\n";
                if(Confirm()){
                    if(!BlockO.GenerateNewBlockChain(newName)){
                        std::cout << "Failed to generate new blockchain\n";
                        break;
                    }

                    if(!BlockO.ExportBlockChain(database)){
                        std::cout << "Failed export blockchain\n";
                        break;
                    }

                    BlockO.ExportKeys(publickey, privatekey);
                }
                break;
            }
        }

        { // generate a new keypair
            std::string name;
            if(FindParam("newkey", name)){
                std::cout << "Generating New Keypair...\n";

                BlockO.GenerateNewKeypair();
                if(!BlockO.ExportKeys(name + ".pub", name + ".key")){
                    std::cout << "failed to generate new keypair\n";
                }

                break;
            }
        }

        if(FindParam("database", database, 1)){
            std::cout << "Warning: A separate blockchain database has been selected\n";
        }
        
        // Import Blockchain Database
        if(!BlockO.ImportBlockChain(database)){
            std::cout << "Failed to import main blockchain database!\n";
            break;
        }

        std::cout << "---------------------------------------------\n";
        
        { // load private key
            std::string privkey;
            if(FindParam("key", privkey)){
                std::cout << "Loading private key...\n";
                if(!BlockO.ImportKey(privkey, PK_PRIVATE)){
                    std::cout << "Failed to import private key!\n";
                }
            }
        }

        {
            std::string index, data, key;
            { // load owner public key
                std::string newowner;
                if(FindParam("ownerkey", newowner)){
                    std::cout << "Loading new owner...\n";
                    key = LoadFileData(newowner);
                    if(key.empty()){
                        std::cout << "Failed to load owner key!\nDo you want to continue?\n";
                        if(!Confirm()) break;
                    }
                    if(key.size() > 1024){
                        std::cout << "Warning: The new owner key is larger than normal, and may be invalid.\nDo you want to continue?\n";
                        if(!Confirm()) break;
                    }
                }
            }
            if(FindParam("addblock", index, 1) && FindParam("addblock", data, 2)){
                std::cout << "Inserting new block into blockchain...\n";
                int64_t id;
                if(!ToInteger(index, id)){
                    std::cout << "Failed because of an invalid index value\n";
                    break;
                }

                Block bfrom;
                if(!BlockO.FindBlock(id, bfrom)){
                    std::cout << "Failed because the stem block doesn't exist!\n";
                    break;
                }
                if(BlockO.CreateBlock(bfrom, key, data)){
                    std::cout << "New block was successfully added to blockchain!\n";

                    std::cout << "Updating blockchain database...\n";
                    if(!BlockO.ExportBlockChain(database)){
                        std::cout << "Failed export blockchain database\n";
                    }
                } else {
                    std::cout << "Failed to add new block to the chain!\n";
                }
            }
        }

        if(FindArg("printchain")){
            for(const Block& block : BlockO.GetBlockChain()){
                Blockchain::PrintBlock(block);
            }
        }

        {
            std::string index;
            if(FindParam("printblock", index, 1)){
                int64_t id;
                if(!ToInteger(index, id)){
                    std::cout << "Failed because of an invalid index value\n";
                    break;
                }
                Block block;
                if(!BlockO.FindBlock(id, block)){
                    std::cout << "Could not find block\n";
                    break;
                }
                Blockchain::PrintBlock(block);
            }
        }
    } while(0);

    std::cout << "---------------------------------------------\n";
    return 0;
}