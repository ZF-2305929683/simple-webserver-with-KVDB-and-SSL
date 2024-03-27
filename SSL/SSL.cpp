#include "SSL.h"

simple_SSL::NetworkType simple_SSL::Get_Networktype() const
{
    return type_;
}


////////////////////////////////////////////////////////////AES加密算法
bool simple_SSL::generate_AES_key_iv()
{
	RAND_bytes(AES_key_.get(), 16);            // 暂时只支持16字节密钥
	RAND_bytes(AES_iv_.get(), AES_BLOCK_SIZE); // AES_BLOCK_SIZE通常是16字节
	return true;
}


std::string simple_SSL::AES_encrypt_EVP(std::span<unsigned char> plaintext)
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	const EVP_CIPHER* cipher = EVP_aes_128_cbc();
	if (EVP_EncryptInit_ex(ctx, cipher, NULL, AES_key_.get(), AES_iv_.get()) != 1)
	{
		std::cerr << "加密初始化失败" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return std::string();
	}
	uint32_t ciphertext_len = 0;
	uint32_t final_len;
	// 创建密文字符串，预留空间容纳额外填充的数据
	std::string ciphertext;
	ciphertext.resize(plaintext.size() + AES_BLOCK_SIZE);

	if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()), reinterpret_cast<int*>(&ciphertext_len), plaintext.data(), plaintext.size()) != 1)
	{
		std::cerr << "加密更新阶段失败" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return std::string();
	}
	
	// 获取最终加密后数据大小
	if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&ciphertext[ciphertext_len]), reinterpret_cast<int*>(&final_len)) != 1)
	{
		std::cerr << "加密结束阶段失败" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return std::string();
	}
	
	ciphertext.resize(ciphertext_len + final_len);
	EVP_CIPHER_CTX_free(ctx);
	return ciphertext;
}

std::string simple_SSL::AES_decrypt_EVP(std::span<unsigned char> ciphertext)
{
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	const EVP_CIPHER* cipher = EVP_aes_128_cbc();
	if (EVP_DecryptInit_ex(ctx, cipher, NULL, AES_key_.get(), AES_iv_.get()) != 1) {
		std::cerr << "解密初始化失败" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return std::string();
	}
	uint32_t decrypted_len = 0;
	uint32_t final_len;
	// 创建保存解密字符串，预留与密文同样大小的空间来解密
	std::string decrypted_text;
	decrypted_text.resize(ciphertext.size());
	if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decrypted_text.data()), reinterpret_cast<int*>(&decrypted_len), ciphertext.data(), ciphertext.size()) != 1) {
		std::cerr << "解密更新阶段失败" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return std::string();
	}
	// 解密结束阶段，并获取最终解密文本的实际长度
	if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&decrypted_text[decrypted_len]), reinterpret_cast<int*>(&final_len)) != 1) {
		std::cerr << "解密结束阶段失败" << std::endl;
		EVP_CIPHER_CTX_free(ctx);
		return std::string();
	}
	decrypted_text.resize(decrypted_len + final_len);
	EVP_CIPHER_CTX_free(ctx);
	return decrypted_text;
}

bool simple_SSL::AES_init()
{
	return generate_AES_key_iv();
}
 
void simple_SSL::AES_set_key(const unsigned char* key)
{
    AES_key_ = std::make_unique<unsigned char[]>(16);
    std::memcpy(AES_key_.get(), key, 16);
}

void simple_SSL::AES_set_iv(const unsigned char* iv)
{
    AES_iv_ = std::make_unique<unsigned char[]>(AES_BLOCK_SIZE);
    std::memcpy(AES_iv_.get(), iv, AES_BLOCK_SIZE);
}



////////////////////////////////////////////////////////////////SHA hash计算
void simple_SSL::to_hex_string(const unsigned char* data, size_t len,std::string &hex)  //将字符串转换为16进制
{
    std::stringstream ss;
    ss << std::hex << std::uppercase;

    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }

    hex = ss.str();
}

std::string simple_SSL::calculate_SHA256(std::span<unsigned char> input)                      //计算哈希值 
{
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();

    if (mdctx == nullptr) 
	{
        std::cout<<"Failed to initialize SHA256 context or get SHA256 method."<<"\n";
        return "";
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
	
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(mdctx, input.data(), input.size()) != 1 ||
        EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) 
	{
        EVP_MD_CTX_free(mdctx);
        std::cout<<"Failed to calculate MD5 digest."<<"\n";
		return "";
    }

    EVP_MD_CTX_free(mdctx);

    // 直接将SHA摘要转换为十六进制字符串
	std::string hex_hash;
	to_hex_string(hash, hash_len, hex_hash);

    return hex_hash;
}     



////////////////////////////////////////////////////////////RSA加密算法

std::string simple_SSL::RSA_encrypt(std::span<unsigned char> plaintext,std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key) 
{
	if (key == NULL) {
		fprintf(stderr, "error: key is null\n");
		return {};
	}
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key.get(), NULL);
	EVP_PKEY_encrypt_init(ctx);
	unsigned char* dst = (unsigned char*)malloc(2048);
	size_t outl;
	if (!EVP_PKEY_encrypt(ctx, dst, &outl, plaintext.data(), plaintext.size())) {
		fprintf(stderr, "error: encrypt\n");
		free(dst);
		return {};
	}
	int len2 = outl;
	EVP_PKEY_CTX_free(ctx);
    std::string result((char*)dst, len2);
    free(dst);
	return result;
}
std::string simple_SSL::RSA_decrypt(std::span<unsigned char> plaintext,std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key)
{
	if (key == NULL) {
		fprintf(stderr, "error: key is null\n");
		return NULL;
	}
	EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key.get(), NULL);
	EVP_PKEY_decrypt_init(ctx);
	unsigned char* dst = (unsigned char*)malloc(2048);
	size_t outl;
	if (!EVP_PKEY_decrypt(ctx, dst, &outl, plaintext.data(), plaintext.size())) {
		fprintf(stderr, "error: decrypt\n");
		free(dst);
		dst = NULL;
	}
	EVP_PKEY_CTX_free(ctx);
	std::string result((char*)dst, outl);
	free(dst);
	return result;
}

bool simple_SSL::RSA_init()
{
    FILE* public_fp = fopen("public.txt", "r");
	if (public_fp == NULL) {
		perror("file error");
		return false;
	}
	RSA_public_key.reset(PEM_read_PUBKEY(public_fp, NULL, NULL, NULL));
	fclose(public_fp);
    FILE* private_fp = fopen("private.txt", "r");
	if (private_fp == NULL) {
		perror("file error");
		return false;
	}
	RSA_private_key.reset(PEM_read_PrivateKey(private_fp, NULL, NULL, NULL));
	fclose(private_fp);
}