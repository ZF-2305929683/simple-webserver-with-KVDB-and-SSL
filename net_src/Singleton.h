#pragma once

template<typename T>
class Singleton {
public:
    [[nodiscard]]
    static T& Instance() {
        static T instance; //static局部变量只初始化一次
        return instance;
    }

private:
    Singleton() = default; // 防止外部实例化
    Singleton(const Singleton&) = delete; // 禁用拷贝构造函数
    Singleton& operator=(const Singleton&) = delete; // 禁用赋值操作符
};