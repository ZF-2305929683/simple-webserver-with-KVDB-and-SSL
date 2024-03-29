#include "simple KVdb/skiplist.h"
#include "simple KVdb/BloomFliter.h"
#include "Singleton.h"
#include <string>
#include "net_src/ThreadPool.h"
#include "byteSerialize/byteSerializer.h"
#include <iostream>
struct node
{
  int key;
  std::string value;
};


template<>
struct TypeInfo<node> :TypeInfoBase<node>
{
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
        Field {register("K"), &Type::key},
        Field {register("V"), &Type::value},
    };
};


int main() 
{
  

}