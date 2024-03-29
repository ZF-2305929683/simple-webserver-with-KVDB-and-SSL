#pragma once
#include <string>
#include <iostream>
#include <list>
#include <sstream>
#include <memory>
#include <condition_variable>
class GlobalCommandList
{
public:
	GlobalCommandList() :Client_(std::make_shared<std::list<std::string>>()) {}
	~GlobalCommandList() = default;

	void Insert(std::string& str)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		Client_->push_back(str);
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
		return 0;
	}
private:
	std::shared_ptr<std::list<std::string>> Client_;
	std::mutex mtx_;
	std::condition_variable cv_;
};