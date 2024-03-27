#include "simple KVdb/skiplist.h"
#include "simple KVdb/BloomFliter.h"
#include "Singleton.h"
#include <string>
#include "net_src/ThreadPool.h"
int main() 
{
  ThreadPool thpoll(3);
  for (int i = 0; i < 3; ++i) {
        auto result_future = thpoll.add([](int id) {
            std::cout << "Processing task " << id << std::endl;
            // 模拟一些耗时操作
            while(1);
        }, i);
    }
}