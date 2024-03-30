#pragma once
#include <string.h>
#include <iostream>
#include <span>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <memory>
#include <cstring>
#include <functional>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include "../net_src/Connection.h"
#include "../byteSerialize/byteSerializer.h"
#include "../net_src/Buffer.h"

struct SSL_Struct
{
    std::string AES_key_;
    std::string AES_iv_;
    std::string RSA_public_key;
    std::string msg;
}; 

template<>
struct TypeInfo<SSL_Struct> :TypeInfoBase<SSL_Struct>
{
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
        Field {register("AES_key_"), &Type::AES_key_},
        Field {register("AES_iv_"), &Type::AES_iv_},
        Field {register("RSA_public_key"), &Type::RSA_public_key},
        Field {register("msg"), &Type::msg},
    };
};

enum class NetworkType { Client, Server };

class simple_SSL
{
private:
    std::unique_ptr<unsigned char[]> AES_key_ = std::make_unique<unsigned char[]>(16);
	std::unique_ptr<unsigned char[]> AES_iv_ = std::make_unique<unsigned char[]>(AES_BLOCK_SIZE);
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> RSA_public_key = {nullptr, EVP_PKEY_free};
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> RSA_private_key = {nullptr, EVP_PKEY_free};

    NetworkType type_;

    //////////////////////////////////////////////////////////////////////////////////////简化状态机
    enum class ClientStateMachine { ClientHello, KeyExchange, Finish, WaitingMessage } ClientState_;

    enum class ServerStateMachine { ServerHello, Finish, WaitingMessage, wait_Finish} ServerState_;

    bool hand_shake_complete = false;
private:
    //转换字符串类型
    template <typename T>
	bool processString(T&& input, std::string& output,
                       std::function<std::string(std::span<unsigned char>)> cb);

    //SHA hash计算
    
    void to_hex_string(const unsigned char* data, size_t len,std::string &hex);//将字符串转换为16进制

    //AES加密
    bool AES_init();

	std::string AES_encrypt_EVP(std::span<unsigned char> plaintext);

	std::string AES_decrypt_EVP(std::span<unsigned char> ciphertext);

    bool generate_AES_key_iv();


    //RSA加密
    bool RSA_init();

    std::string RSA_encrypt(std::span<unsigned char> plaintext,
                            std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key);

    std::string RSA_decrypt(std::span<unsigned char> plaintext,
                            std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key);



    
    //////////////////////////client初始化与简化握手
    bool client_init(){
        ClientState_ = ClientStateMachine::ClientHello;
        return AES_init();
    }

    void Clienthello(Connection* conn,SSL_Struct& ssl,ByteStream& stream);

    void KeyExchange(Connection* conn,SSL_Struct& ssl,ByteStream& stream);

    void client_WaitingMessage(Connection* conn,SSL_Struct& ssl);

    void Client_do_handshake(Connection* conn,SSL_Struct& ssl,ByteStream& stream);

    bool client_hand_shake(){return ClientState_ != ClientStateMachine::Finish;};


    

    //////////////////////////server初始化与简化握手
    bool server_init(){
        ServerState_ = ServerStateMachine::WaitingMessage;
        return RSA_init();
    }

    void ServerHello(Connection* conn,SSL_Struct& ssl,ByteStream& stream);

    void wait_Finish(Connection* conn,SSL_Struct& ssl,ByteStream& stream);

    void server_WaitingMessage(Connection* conn,SSL_Struct& ssl);

    void Server_do_handshake(Connection* conn,SSL_Struct& ssl,ByteStream& stream);

    bool server_hand_shake(){return ServerState_ != ServerStateMachine::Finish;};
    
public:
    simple_SSL(NetworkType type):type_(type){
        if(Init() == false) perror("Fail to Init simple_ssl");
    };
    simple_SSL& operator= (simple_SSL& other) = delete;
    simple_SSL& operator= (simple_SSL&& other) = delete;
    simple_SSL(const simple_SSL& other) = delete;
    simple_SSL(const simple_SSL&& other) = delete;
    ~simple_SSL() {};

    bool Init(){
        if(type_ == NetworkType::Client) return client_init();
        else if(type_ == NetworkType::Server) return server_init();
        return false;
    }

    bool hand_shake(){
        if(type_ == NetworkType::Client) return client_hand_shake();
        else if(type_ == NetworkType::Server) return server_hand_shake();
        return false;
    }

    void do_hand_shake(Connection* conn,SSL_Struct& ssl,ByteStream& stream){
        if(type_ == NetworkType::Client) Client_do_handshake(conn,ssl,stream);
        else if(type_ == NetworkType::Server) Server_do_handshake(conn,ssl,stream);
    }

    NetworkType Get_Networktype() const;

    //SHA hash计算
    std::string calculate_SHA256(std::span<unsigned char> input);              //计算哈希值

    //AES加密
    template <typename T>
	bool AES_encryptString(T&& input, std::string& ciphertext);

	template <typename T>
	bool AES_decryptString(T&& input, std::string& decrypted_text);

    
 
	void AES_set_key(const unsigned char* key);

	void AES_set_iv(const unsigned char* iv);


    //RSA加密

    template <typename T>
	bool RSA_client_encryptString(T&& input, std::string& ciphertext);

	template <typename T>
	bool RSA_client_decryptString(T&& input, std::string& decrypted_text);

    template <typename T>
	bool RSA_server_encryptString(T&& input, std::string& ciphertext);

	template <typename T>
	bool RSA_server_decryptString(T&& input, std::string& decrypted_text);

};



template <typename T>
bool simple_SSL::AES_encryptString(T&& input, std::string& ciphertext)
{
	processString(std::forward<T>(input), ciphertext,[this](std::span<unsigned char> span) {
		return AES_encrypt_EVP(span);
		});
	return !ciphertext.empty();
}

template <typename T>
bool simple_SSL::AES_decryptString(T&& input, std::string& decrypted_text)
{
	processString(std::forward<T>(input), decrypted_text,[this](std::span<unsigned char> span) 
    {
		return AES_decrypt_EVP(span);
	});
	return !decrypted_text.empty();
}

template <typename T>
bool simple_SSL::RSA_client_encryptString(T&& input, std::string& ciphertext)
{
    processString(std::forward<T>(input),ciphertext,[this](std::span<unsigned char> span)
    {
        return RSA_encrypt(span,RSA_public_key);
    });
    return !ciphertext.empty();
}

template <typename T>
bool simple_SSL::RSA_client_decryptString(T&& input, std::string& decrypted_text)
{
    processString(std::forward<T>(input),decrypted_text,[this](std::span<unsigned char> span)
    {
        return RSA_decrypt(span,RSA_public_key);
    });
    return !decrypted_text.empty();
}

template <typename T>
bool simple_SSL::RSA_server_encryptString(T&& input, std::string& ciphertext)
{
    processString(std::forward<T>(input),ciphertext,[this](std::span<unsigned char> span)
    {
        return RSA_encrypt(span,RSA_private_key);
    });
    return !ciphertext.empty();
}

template <typename T>
bool simple_SSL::RSA_server_decryptString(T&& input, std::string& decrypted_text)
{
    processString(std::forward<T>(input),decrypted_text,[this](std::span<unsigned char> span)
    {
        return RSA_decrypt(span,RSA_private_key);
    });
    return !decrypted_text.empty();
}



template <typename T>
bool simple_SSL::processString(T&& input, std::string& ciphertext,
                   std::function<std::string(std::span<unsigned char>)> cb)
{
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>)
	{
		if ((ciphertext = cb({ reinterpret_cast<unsigned char*>(input.data()), input.size() })).empty()) return false;
		else return true;
	}
	else if constexpr (std::is_same_v<std::remove_extent_t<std::remove_cvref_t<T>>, unsigned char>)
	{
		if ((ciphertext = cb({ input, strlen(reinterpret_cast<const char*>(input)) })).empty()) return false;
		else return true;
	}
	else if constexpr (std::is_same_v<std::remove_pointer_t<std::remove_cvref_t<T>>, unsigned char>)
	{
		if ((ciphertext = cb({ input, strlen(reinterpret_cast<const char*>(input)) })).empty()) return false;
		else return true;
	}
	else if constexpr (std::is_same_v<std::remove_extent_t<std::remove_cvref_t<T>>, char>)
	{
		if ((ciphertext = cb({ reinterpret_cast<unsigned char*>(input), strlen(input) })).empty()) return false;
		else return true;
	}
	else if constexpr (std::is_same_v<std::remove_pointer_t<std::remove_cvref_t<T>>, char>)
	{
		if ((ciphertext = cb({ reinterpret_cast<unsigned char*>(input), strlen(input) })).empty()) return false;
		else return true;
	}
	else
	{
		std::cout << "Unsupported string type" << "\n";
		return false;
	}
}