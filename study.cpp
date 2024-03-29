#include "simple KVdb/skiplist.h"
#include "simple KVdb/BloomFliter.h"
#include "Singleton.h"
#include <string>
#include "net_src/ThreadPool.h"
#include "byteSerialize/byteSerializer.h"
#include <iostream>
#include <list>
#include <sstream>

class GlobalCommandList
{
public:
	GlobalCommandList() :Client_(std::make_shared<std::list<std::string>>()) {}
	~GlobalCommandList() = default;

	void Insert(std::string& str)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		Client_->push_back(str);
		std::cout << "insert" << "\n";
		cv_.notify_all();
	}

	size_t CommandParsing(std::string &Command,std::string &key,std::string &value)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.wait(lock, [this] { return !Client_->empty(); });
		std::string str = std::move(Client_->front());
		Client_->pop_front();
		if (str == "")
		{
			return -1;
		}
  
		std::istringstream iss(str);
		iss >> Command;
		iss >> key;
		iss >> value;
		std::cout << "CommandParsing" << "\n";
    std::cout << Command << "\n";
    std::cout << key << "\n";
    std::cout << value << "\n";
		return 0;
	}
private:
	std::shared_ptr<std::list<std::string>> Client_;
	std::mutex mtx_;
	std::condition_variable cv_;
};


int main() 
{
  SkipList<std::string,std::string> &KV_DB = Singleton<SkipList<std::string,std::string>>::Instance(3);
  GlobalCommandList &global_list = Singleton<GlobalCommandList>::Instance();
  int times = 3;
  std::thread readAnd_Do([&]() {
    while (times >0) {
        std::string Command;
        std::string key;
        std::string value;
        

        global_list.CommandParsing(Command,key,value);

        if(Command == "Insert") KV_DB.insert_element(key,value);
        else{
          std::cout<<"命令错误"<<"\n";
        }

        times -= 1;
    }
});

  std::string str = "Insert 1 hah";
  std::string str1 = "Insert 2 kdis";
  std::string str2 = "Insert 3 vdfhsjbf";

  std::thread insert([&]() {
    global_list.Insert(str);
  });
   std::thread insert1([&]() {
    global_list.Insert(str1);
  });
   std::thread insert2([&]() {
    global_list.Insert(str2);
  });

  if(readAnd_Do.joinable()) readAnd_Do.join();

  if(insert.joinable()) insert.join();
  if(insert1.joinable()) insert1.join();
  if(insert2.joinable()) insert2.join();
  
  KV_DB.display_list();
  KV_DB.delete_element("4");
  KV_DB.search_element("4");
  KV_DB.delete_element("3");
  KV_DB.display_list();
  KV_DB.search_element("3");
  KV_DB.search_element("1");
}