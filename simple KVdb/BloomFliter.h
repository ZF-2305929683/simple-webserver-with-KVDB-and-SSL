#include<string>
#include<bitset>
class BloomFliter
{
public:
	BloomFliter() {};

	void add(const std::string& str) 
	{
		bitArray[hashFunction1(str) % 100000] = 1;
	}

	bool find(const std::string& str) 
	{
		return bitArray[hashFunction1(str) % 100000] == 1;
	}


	~BloomFliter(){};

private:
	std::bitset<100000> bitArray;
	std::hash<std::string> hashFunction1;
};