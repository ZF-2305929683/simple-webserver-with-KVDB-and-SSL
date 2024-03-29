#pragma once
#include<vector>
#include<cstdint>
#include<string>
#include<cstring>
#include<iostream>
#include"static_reflaction.h"

class ByteStream
{
private:
    std::vector<uint8_t> buffer_;

public:
    /// @brief 默认构造函数，拷贝构造与移动构造函数
    ByteStream(){}
    ByteStream(const std::vector<uint8_t>& data):buffer_(data){}

    ByteStream(const char* data){
        size_t dataSize = strlen(data);
        buffer_.reserve(dataSize); // 预先分配空间
        buffer_.resize(dataSize); // 改变容器大小并初始化元素（可选，取决于你的需求）
        std::copy(data, data + dataSize, buffer_.begin());
    }

    ByteStream(const char* data,size_t len){
        buffer_.reserve(len); // 预先分配空间
        buffer_.resize(len); // 改变容器大小并初始化元素（可选，取决于你的需求）
        std::copy(data, data + len, buffer_.begin());
    }

    ByteStream(ByteStream&& other):buffer_(std::move(other.buffer_)){}
    ByteStream& operator=(const ByteStream& other){
        buffer_.assign(other.buffer_.begin(), other.buffer_.end());
        return *this;
    }
    ByteStream& operator=(ByteStream&& other) noexcept {
        buffer_.clear();
        buffer_ = std::move(other.buffer_);
        return *this;
    }

    void Clear(){
        buffer_.clear();
    }
    void append(const void* data, size_t length) {
        buffer_.insert(buffer_.end(), (uint8_t*)data, (uint8_t*)data + length);
    }

    void append(const std::string& str) {
        append(str.size());
        append(str.data(), str.size());
    }

    template <typename T>
    void append(const T& data) {
        static_assert(std::is_standard_layout<T>::value && std::is_trivial<T>::value, "Only POD types are supported.");
        append(reinterpret_cast<const void*>(&data),sizeof(T));
    }

    template<typename T>
    void append(const std::vector<T>& invector) {
        append(invector.size());
        for(const T& element:invector){
            append(element);
        }
    }

    template <typename T>
    void read(T& t) {
        static_assert(std::is_standard_layout<T>::value && std::is_trivial<T>::value, "Only POD types are supported.");
        if (size() < sizeof(T)) {
            throw std::runtime_error("Insufficient data in the ByteStream to deserialize a T object.");
        }
        memcpy(&t,buffer_.data(),sizeof(T));
        buffer_.erase(buffer_.begin(),buffer_.begin()+sizeof(T));
    }

    void read(std::string& str){
        size_t len;
        read(len);
        if (size() < len) {
            throw std::runtime_error("Insufficient data in the ByteStream to deserialize a T object.");
        }
        str.resize(len);
        memcpy(str.data(),buffer_.data(),len);
        buffer_.erase(buffer_.begin(),buffer_.begin()+len);
    }

    template<typename T>
    void read(std::vector<T>& vec) {
        size_t len;
        read(len);
        if (size() < len) {
            throw std::runtime_error("Insufficient data in the ByteStream to deserialize a T object.");
        }
        for(int i = 0;i<len;++i){
            T tmp;
            read(tmp);
            vec.push_back(tmp);
        }
    }

    std::vector<uint8_t> extract(size_t count) {
        std::vector<uint8_t> data(buffer_.begin(), buffer_.begin() + count);
        buffer_.erase(buffer_.begin(), buffer_.begin() + count);
        return data;
    }

    std::vector<uint8_t> extractAll() {
        std::vector<uint8_t> data(buffer_);
        buffer_.clear();
        return data;
    }

    bool empty() const {
        return buffer_.empty();
    }

    size_t size() const {
        return buffer_.size();
    }

    std::vector<uint8_t> get_buffer() const{
        return buffer_;
    }
};

template<typename T>
void Serlize(ByteStream& stream,T &t){
    TypeInfo<T>::ForEachVarOf(t, [&](const auto& field,auto var) {
        stream.append(var);
    });
}

template<typename T>
void Deserlize(ByteStream& stream,T &t){
    TypeInfo<T>::ForEachVarOf(t, [&](const auto& field,auto& var) {
        stream.read(var);
    });
}




/// 使用示例
/*struct Vec{
    float x;
    float y;
    std::string str;
    std::vector<int> vx;
    float norm() const noexcept {
        return std::sqrt(x * x + y * y);
    }
};

template<>
struct TypeInfo<Vec> :TypeInfoBase<Vec>
{
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
        Field {register("x"), &Type::x},
        Field {register("y"), &Type::y},
        Field {register("str"), &Type::str},
        Field {register("vx"), &Type::vx},
    };
};*/

