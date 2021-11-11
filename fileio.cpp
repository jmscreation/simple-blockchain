#include "fileio.h"

DataManipulator::DataManipulator(): readonly(false), error(false), pos(0), length(0), wdata( std::make_unique<std::stringstream>() ) {}
DataManipulator::DataManipulator(const char* data, size_t length):
                                    readonly(true), error(false), pos(0), length(length), rdata(data) {}


bool DataManipulator::readString(std::string& rval) {
    if(!readonly) return false;
    size_t sz;
    if(!readData(sz)){
        return false;
    }

    if(pos + sz - 1 >= length){
        error = true;
        return false;
    }

    rval.assign(rdata + pos, sz);
    
    pos += sz;
    return true;
};

bool DataManipulator::writeString(const std::string& rval) {
    if(readonly) return false;
    
    writeData(rval.size());
    return wdata->write(rval.data(), rval.size()).good();
};