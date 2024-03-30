#include "simple KVdb/skiplist.h"
#include "simple KVdb/BloomFliter.h"
#include "Singleton.h"
#include <string>
#include "net_src/ThreadPool.h"
#include "byteSerialize/byteSerializer.h"
#include <iostream>
#include <list>
#include <sstream>
#include "SSL/SSL.h"

int main() 
{
  simple_SSL Server(NetworkType::Server);
  simple_SSL Client(NetworkType::Client);

  
}