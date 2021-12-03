#pragma once

#include <sstream>
#include <memory>
#include <cstring>

class DataManipulator {

    bool readonly, error;
    size_t pos, length;

    union {
        const char* rdata;
        std::unique_ptr<std::stringstream> wdata;
    };

public:
    bool readString(std::string& rval);
    bool writeString(const std::string& rval);


    template<class T>
    bool readData(T& rval) {
        if(!readonly) return false;

        int sz = sizeof(rval);

        if(pos + sz - 1 >= length){
            error = true;
            return false;
        }

        memcpy(reinterpret_cast<char*>(&rval), rdata + pos, sz);
        
        pos += sz;
        return true;
    };

    template<class T>
    bool writeData(const T& rval) {
        if(readonly) return false;
        int sz = sizeof(rval);

        return wdata->write(reinterpret_cast<const char*>(&rval), sz).good();
    };


    inline bool exportData(std::stringstream& stream) { // append data to stream
        if(readonly) return false;

        return (stream << wdata->rdbuf()).good();
    }

    DataManipulator(); // writer
    DataManipulator(const char* data, size_t length); // reader

    virtual ~DataManipulator() {}
};

