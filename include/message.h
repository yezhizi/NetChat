#ifndef _MESSAGE_H
#define _MESSAGE_H
#include "sarray.h"
#include <vector>
#include "base.h"
#include "meta.pb.h"
struct Meta {
    using Type = ntc::Type;
    using Code = ntc::Code;
    Meta() : type(Type::TYPE_UNKNOWN), src(ntc::kvEmpty), dst(ntc::kvEmpty), code(Code::CODE_UNKNOWN), data_size({}) {}

    Type type;
    Code code;
    int dst;
    int src;
    std::vector<int> data_size;
};
struct Message {

    Meta meta;
    std::vector<SArray<char>> data;

    template<typename V>
    void AddData(const SArray<V>& val) {
        SArray<char> bytes(val);
        meta.data_size.push_back(bytes.size());
        data.push_back(bytes);
    }
};



#endif //_MESSAGE_H