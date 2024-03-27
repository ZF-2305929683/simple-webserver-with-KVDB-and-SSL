#include <string_view>                
#include <tuple> 
#include <iostream>
#include <functional>
#include <cmath>


//////////////////宏
//////////////////创建保存成员变量信息的结构体
#define register(x) ([]{                    \
    constexpr std::basic_string_view str{x};\
    return static_str<cstr_Struct<typename decltype(str)::value_type,str.size()>{str}>{};\
}())


//////////////////保存成员变量信息结构体 cstr_Struct
template<typename type,std::size_t N>
struct cstr_Struct{
    using value_type = type;
    value_type data[N+1]{};
    static constexpr std::size_t size = N;
    constexpr cstr_Struct(std::basic_string_view<value_type> str){
        for(std::size_t i = 0;i<size;++i) data[i] = str[i];
    }
};

//////////////////创建一个静态类包装 cstr_Struct 结构体
template<cstr_Struct str>
struct static_str
{
    using Char = typename decltype(str)::value_type;
    static constexpr auto Data() {
        return str.data;
    }
    static constexpr auto size(){
        return str.size;
    }
    static constexpr std::basic_string_view<Char> View(){
        return str.data;
    }
};


///////////////////////////递归访问成员
///////////////////终止条件
template<typename Filedlists,typename Function,typename ReturnType>
constexpr auto Iterate_acess(const Filedlists&,Function&&,ReturnType var,std::index_sequence<>){return var;}
///////////////////递归主体
template<typename Filedlists,typename Function,typename ReturnType,std::size_t N0,std::size_t... Ns>
constexpr auto Iterate_acess(const Filedlists& fileds,Function&& function,ReturnType var,std::index_sequence<N0,Ns...>){
    return Iterate_acess(fileds,std::forward<Function>(function),
    function(std::move(var),fileds.template Get<N0>()),std::index_sequence<Ns...>{});
}


template<typename TI, typename U, typename Function> 
constexpr void NV_Var(TI, U&& u, Function&& function) {
    TI::fields.ForEach([&](auto&& k){
        using K=std::decay_t<decltype(k)>;
        if constexpr(!K::is_static&&!K::is_func)
           std::forward<Function>(function)(k,std::forward<U>(u).*(k.value_));
        });
  }

//////////////////////////////////////////////////////递归查找成员
/// <brief> 递归查找成员的终止条件。
/// <tparam Filedlists,Function> Filedlists，结构体类型。Function，函数类型，可以是lambda也可以是std::function。
/// <param name = ""> 终止条件只需要空的参数列表。
template<typename Filedlists,typename Function>
constexpr std::size_t Find_operator(const Filedlists&,Function&&,std::index_sequence<>){
    return -1;
}
/// <brief> 递归查找成员的递归主体。
/// <tparam Filedlists,Function> Filedlists，结构体类型。Function，函数类型，可以是lambda也可以是std::function。
/// <param name = "filed，function，std::index_sequence<N0,Ns...>"> filed保存信息的结构体，function自定义处理函数，std::index_sequence<N0,Ns...>filed中剩余数据个数。
template<typename Filedlists,typename Function,std::size_t N0,std::size_t... Ns>
constexpr std::size_t Find_operator(const Filedlists& fileds,Function&& function,std::index_sequence<N0,Ns...>){
    return function(fileds.template Get<N0>()) ? N0 : Find_operator(fileds,std::forward<Function>(function),std::index_sequence<Ns...>{});
}

///////////////////////////////////////////////////////保存注册信息结构体,包含一些访问方法
template<typename...Elements>
struct ElemList{
    std::tuple<Elements...> elements_;
    static constexpr std::size_t size = sizeof...(Elements);

    constexpr ElemList(Elements... elements):elements_(elements...){}
    
    /// <brief> 遍历注册信息，可以自定义函数遍历处理所有成员。
    /// <tparam Function> 函数类型，可以是lambda也可以是std::function。 
    /// <param name = "function"> 自定义处理函数。
    template<typename Function>
    constexpr void ForEach(Function&& function) const{
        Iterate_acess(*this,[&](auto, const auto& field) {
            std::forward<Function>(function)(field); return 0; 
            }
            ,0,std::make_index_sequence<size>{});
    }


    /// <brief> 查找对应的注册信息，可以自定义函数处理对应成员。
    /// <tparam Function> 函数类型，可以是lambda也可以是std::function。 
    /// <param name = "function"> 自定义处理函数。
    template<typename Function>
    constexpr std::size_t Findif(Function&& function) const{
        return Find_operator(*this,std::forward<Function>(function),std::make_index_sequence<size>{});
    }


    /// <brief> 查找对应的注册信息，可以自定义函数处理对应成员。
    /// <tparam static_str> 结构体。 
    /// <param name = "static_str"> static_str是一个静态结构体，提供了访问cstr_Struct内容的方法。
    template<typename static_str>
    constexpr const auto& Find(static_str = {}) const{
        constexpr std::size_t index = [](){
            constexpr decltype(static_str::View()) names[] {Elements::name...};
            for(std::size_t i = 0;i<size;i++){
                if(static_str::View() == names[i]) return i;
            }
            return static_cast<std::size_t>(-1);
        }();
        return Get<index>();
    }

    /// <brief> 查找对应的注册信息，可以自定义函数处理对应成员。
    /// <tparam Filed> 结构体。 
    /// <param name = "value"> value保存变量信息的结构体。
    template<typename Filed>
    constexpr std::size_t FindValue(const Filed& value) const {
        return Findif([&value](auto elem){ return value == elem;});
    }


    /// <brief> 查找对应的注册信息，返回对应变量的指针。
    /// <tparam T,static_str> T变量类型，static_str结构体。 
    /// <param name = "str"> str保存变量名的结构体。
    template<typename T, typename static_str>
    constexpr const T* ValuePtrOfName(static_str str) const{
        return Iterate_acess(*this,[str](auto r,const auto& filed){
            if constexpr(std::is_same_v<decltype(filed.value),T>) return filed.name == str ? &filed.value : r;
            else return nullptr;
        },nullptr,std::make_index_sequence<size>{});
    }

    /// <brief> 查找对应的注册信息，返回对应变量。
    /// <tparam T,static_str> T变量类型，static_str结构体。 
    /// <param name = "str"> str保存变量名的结构体。
    template<typename T, typename static_str>
    constexpr const T& ValueOfName(static_str str) const {
        return *ValuePtrOfName<T>(str);
    }


    /// <brief> 查找对应的注册信息，返回对应变量名。
    /// <tparam T,static_str> T变量类型，static_str结构体。 
    /// <param name = "str"> str保存变量名的结构体。
    template<typename T,typename C = char>
    constexpr auto NameOfValue(const T& value) const{
        return Iterate_acess(*this,[&value](auto r,auto&& filed){
            return filed == value ? filed.name : r;
        },std::basic_string_view<C>{},std::make_index_sequence<size>{});
    }


    /// <brief> 增加管理的成员变量。
    /// <tparam Field> 结构体。 
    /// <param name = "field"> field保存变量信息的结构体。
    template<typename Field>
    constexpr auto Push(Field field) const{
        return std::apply([field](auto...elements){return ElemList<Elements...,Field>{elements...,field};},elements_);
    }

    /// <brief> 如果该类型不存在就增加管理的成员变量。
    /// <tparam Field> 结构体。 
    /// <param name = "field"> field保存变量信息的结构体。
    template<typename Field>
    constexpr auto Insert(Field field) const{
        if constexpr((std::is_same_v<Elements,Field>||...)) return *this;
        else return Push(field);
    }


    /// <brief> 返回对应的变量。
    /// <tparam size_t> 常数。 
    /// <param name = "N"> 对应变量的index。
    template<std::size_t N>
    constexpr const auto& Get() const {return std::get<N>(elements_);}
};

template<typename Name> 
struct NamedValueBase
{
    using TName = Name;
    static constexpr std::string_view name = TName::View();
};

template<typename Name,typename T>
struct NameValue : NamedValueBase<Name>
{
    T value_;
    static constexpr bool has_value = true;

    constexpr NameValue(T v):value_(v){}

    template<typename U>
    constexpr bool operator==(U u) const{
        if constexpr(std::is_same_v<T,U>) return value_ == u;
        else return false;
    }
};

template<typename Name>
struct NameValue<Name,void> : NamedValueBase<Name>
{
    static constexpr bool has_value = false;

    template<typename U>
    constexpr bool operator==(U) const{
        return false;
    }
};

template<typename Name,typename T>
struct Attr : NameValue<Name,T> {
    constexpr Attr(Name,T value):NameValue<Name,T>{value}{}
};

template<typename Name>
struct Attr<Name,void> : NameValue<Name,void> {
    constexpr Attr(Name){};
};

template<typename...As>
struct AttrList:ElemList<As...>{
    constexpr AttrList(As...as):ElemList<As...>{as...}{}
};

template<bool s,bool f>
struct FTraitsB{
    static constexpr bool is_static = s, is_func = f;
};

template<typename T> 
struct FTraits : FTraitsB<true, false> {};


template<typename U, typename T> 
struct FTraits<T U::*> : FTraitsB<false, std::is_function_v<T>> {};

template<typename T> 
struct FTraits<T*> : FTraitsB<true, std::is_function_v<T>>{}; 

template<typename T>
struct variable_type{
    using type = T;
};

template<typename Class,typename T>
struct variable_type<T Class::*>{
    using type = T;
};

template<typename T>
using variable_type_t = typename variable_type<T>::type;

template<typename T>
struct basic_variable_traits{
    using type = variable_type_t<T>;
    static constexpr bool is_member = std::is_member_pointer_v<T>;
};

template<typename T>
struct variable_traits;

template<typename T>
struct variable_traits<T *>:basic_variable_traits<T>{
    using pointer_type = T *;
};

template<typename Class,typename T>
struct variable_traits<T Class::*>:basic_variable_traits<T Class::*>{
    using pointer_type = T Class::*;
    using clazz = Class;
};

template<typename Name,typename T,typename AttrList>
struct Field:FTraits<T>,NameValue<Name,T>{
    AttrList attrlist;
    using type = variable_traits<T>;
    constexpr Field(Name,T value,AttrList al = {}):NameValue<Name,T>{value},attrlist{al}{}

    static constexpr bool is_member = type::is_member;
    using var_type = type::type;
    using pointer_type = type::pointer_type;
    using clazz = type::clazz;

    static constexpr auto var_type_name() noexcept -> const char* {
        return typeid(var_type).name();
    }
};

template<typename Name, typename T> 
Field(Name, T)->Field<Name, T, AttrList<>>;

template<typename...Elements>
struct FieldList:ElemList<Elements...>{
    constexpr FieldList(Elements...elements):ElemList<Elements...>{elements...}{}
};

template<typename T> 
struct TypeInfo;

template<typename T,typename...Fields>
struct TypeInfoBase{
    using Type = T;
    static constexpr FieldList fieldlist{Fields{}...};


    template<typename U, typename Function> 
    static constexpr void ForEachVarOf(U&& obj, Function&& function) {
      NV_Var(TypeInfo<Type>{}, std::forward<U>(obj), std::forward<Function>(function)); 
      }
};

