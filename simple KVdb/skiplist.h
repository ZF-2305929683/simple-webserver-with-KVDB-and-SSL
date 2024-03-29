#pragma once
#include <mutex>
#include <atomic>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "BloomFliter.h"

template<typename K, typename V>
class Node {

public:

    Node() {}

    Node(K k, V v, int);

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);

    Node<K, V>** forward;

    int node_level;

private:
    K key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) 
{
    this->key = k;
    this->value = v;
    this->node_level = level;

    this->forward = new Node<K, V>*[level + 1];

    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
};

template<typename K, typename V>
Node<K, V>::~Node() 
{
    delete[]forward;
};

template<typename K, typename V>
K Node<K, V>::get_key() const 
{
    return key;
};

template<typename K, typename V>
V Node<K, V>::get_value() const 
{
    return value;
};
template<typename K, typename V>
void Node<K, V>::set_value(V value) 
{
    this->value = value;
};










template<typename K,typename V>
class SkipList
{
public:
	SkipList(int maxlevel);
	~SkipList();

    uint32_t get_random_level();
    Node<K, V>* create_node(K k, V v, int level);
    void insert_element(const K key,const V value);
    void display_list();
    bool search_element(K key);
    void delete_element(K k);
    void dump_file();
    void load_file();
    uint32_t size();

    
private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);
    bool is_valid_string(const std::string&& str);
private:
	

	uint32_t max_level;
	uint32_t skiplist_level;


    Node<K, V>* header;
	
	std::ofstream file_writer;
	std::ifstream file_reader;

    uint32_t element_count;
	
	std::mutex lock;

    BloomFliter fliter;
};


template<typename K, typename V>
SkipList<K,V>::SkipList(int maxlevel)
{
    this->max_level = maxlevel;
    this->skiplist_level = 0;
    this->element_count = 0;
    K k;
    V v;
    this->header = new Node<K, V>(k, v, max_level);
}

template<typename K, typename V>
SkipList<K, V>::~SkipList()
{
    if (file_writer.is_open()) 
    {
        file_writer.close();
    }
    if (file_reader.is_open()) 
    {
        file_reader.close();
    }
    delete header;
}

template<typename K, typename V>
uint32_t SkipList<K, V>::get_random_level()
{
    uint32_t k = 1;
    while (rand() % 2) 
    {
        k += 1;
    }
    k = (k < max_level) ? k : max_level;
    return k;
}

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) 
{
    Node<K, V>* node = new Node<K, V>(k, v, level);
    return node;
}

template<typename K, typename V>
void SkipList<K, V>::insert_element(const K key, const V value) 
{
    std::lock_guard<std::mutex> lockguard(lock);
    Node<K, V>* node = this->header;

    Node<K, V>* update[max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (max_level + 1));

    for (int i = skiplist_level; i >= 0; i--) 
    {
        while (node->forward[i] != NULL && node->forward[i]->get_key() < key)
        {
            node = node->forward[i];
        }
        update[i] = node;
    }

    node = node->forward[0];

    if (node != NULL && node->get_key() == key) 
    {
        std::cout << "key:" << key << ",exist" << std::endl;
        return;
    }

    if (node == NULL || node->get_key() != key) 
    {
        int random = get_random_level();

        if (random > skiplist_level) 
        {
            for (int i = skiplist_level + 1; i < random + 1; i++) 
            {
                update[i] = header;
            }
            skiplist_level = random;
        }

        Node<K, V>* insert_node = create_node(key, value, random);

        for (int i = 0; i <= random; i++) 
        {
            insert_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = insert_node;
        }
        fliter.add(key);
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        element_count++;

    }
    return;
}

template<typename K, typename V>
bool SkipList<K, V>::search_element(K key)
{
    if(fliter.find(key) == false) 
    {
        std::cout << "Not Found Key:" << key << std::endl;
        return false;
    }

    Node<K, V>* node = header;

    for (int i = skiplist_level; i >= 0; i--) 
    {
        while (node->forward[i] && node->forward[i]->get_key() < key) 
        {
            node = node->forward[i];
        }
    }

    node = node->forward[0];

    if (node and node->get_key() == key)
    {
        std::cout << "Found key: " << key << ", value: " << node->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

template<typename K, typename V>
void SkipList<K, V>::delete_element(K key) 
{
    if(fliter.find(key) == false) 
    {
        std::cout << "Key " << key << " not found in the skiplist." << std::endl;
        return;
    }

    std::lock_guard<std::mutex> guard(lock);
    Node<K, V>* node = this->header;
    Node<K, V>* update[max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (max_level + 1));

   
    for (int i = skiplist_level; i >= 0; i--) 
    {
        while (node->forward[i] != NULL && node->forward[i]->get_key() < key)
        {
            node = node->forward[i];
        }
        update[i] = node;
    }

    node = node->forward[0];
    if (node != NULL && node->get_key() == key) 
    {

        
        for (int i = 0; i <= skiplist_level; i++) 
        {

           
            if (update[i]->forward[i] != node)
                break;

            update[i]->forward[i] = node->forward[i];
        }

        
        while (skiplist_level > 0 && header->forward[skiplist_level] == 0) 
        {
            skiplist_level--;
        }

        std::cout << "Successfully deleted key " << key << std::endl;
        delete node;
        element_count--;
    }
    else 
    {
        std::cout << "Key " << key << " not found in the skiplist." << std::endl;
    }
    return;
}

template<typename K, typename V>
void SkipList<K, V>::display_list() 
{
    std::cout << "///////SkipList///////" << std::endl;
    for (int i = 0; i <= skiplist_level; i++) 
    {
        Node<K, V>* node = this->header->forward[i];
        std::cout << "level" << i << ":";
        while (node != NULL)
        {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}


template<typename K, typename V>
void SkipList<K, V>::dump_file() 
{
    std::cout << "///////////dump_file//////////////" << std::endl;
    file_writer.open("dumpfile/file");
    Node<K, V>* node = this->header->forward[0];

    while (node != NULL) 
    {
        file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout<< node->get_key() << ":" << node->get_value() << "\n";
        node = node->forward[0];
    }

    file_writer.flush();
    file_writer.close();
    return;
}

template<typename K, typename V>
void SkipList<K, V>::load_file()
{
    file_reader.open("dumpfile/file");
    std::cout << "///////////load_file//////////////" << std::endl;
    
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();

    while (getline(file_reader,line))
    {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) continue;
        insert_element(stoi(*key), *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    
    delete key;
    delete value;
    file_reader.close();
    return;
}

template<typename K, typename V>
uint32_t SkipList<K, V>::size() 
{
    return element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) 
{
    if (!is_valid_string(str)) 
    {
        return;
    }
    *key = str.substr(0, str.find(":"));
    *value = str.substr(str.find(":") + 1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) 
{

    if (str.empty()) 
    {
        return false;
    }
    if (str.find(":") == std::string::npos) 
    {
        return false;
    }
    return true;
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string&& str)
{

    if (str.empty())
    {
        return false;
    }
    if (str.find(":") == std::string::npos)
    {
        return false;
    }
    return true;
}
