#pragma once

template<typename T,typename... Args>
class Singleton {
public:

    template<typename... Ts>
    static T& Instance(Ts&&... params) {
        static T instance(std::forward<Ts>(params)...);
        return instance;
    }

private:
    Singleton() = default; // 防止外部实例化
    Singleton(const Singleton&) = delete; // 禁用拷贝构造函数
    Singleton& operator=(const Singleton&) = delete; // 禁用赋值操作符
};