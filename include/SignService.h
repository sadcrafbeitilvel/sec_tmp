#include <openssl/des.h>
#include <openssl/md5.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

class SignService {
private:
    string key = "nhfdt24gjgw3jypn";
    string iv = "nhfdt24gjgw3jypn";
public:
    map<string, string> signSend(const map<string, string>& input) {
        map<string, string> output;
        
        string serializedInput;
        for (const auto& pair : input) {
            serializedInput += pair.first + "=" + pair.second + "\n";
        }        

        DES_key_schedule desKey;
        DES_set_key((const_DES_cblock*)key.c_str(), &desKey);
        string ciphertext(serializedInput.length(), '\0');
        DES_ncbc_encrypt((const unsigned char*)serializedInput.c_str(), (unsigned char*)ciphertext.data(), serializedInput.length(), &desKey, (DES_cblock*)iv.c_str(), DES_ENCRYPT);
        output["ciphertext"]=ciphertext;
        cout << "signSend ciphertext: " << ciphertext << endl;

        char salt[8];
        srand(time(nullptr)); 
        for (int i = 0; i < 8; i++) {
            salt[i] = static_cast<char>(std::rand() % 256);
        }
        string salt_str(salt, 8);
        output["salt"] = salt_str;
        ciphertext += salt_str;
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)ciphertext.c_str(), ciphertext.length(), digest);
        char md5string[33];
        for (int i = 0; i < 16; i++) {
            sprintf(&md5string[i*2], "%02x", (unsigned int)digest[i]);
        }
        output["sign"]=md5string;
        
        return output;
    }

    map<string, string> signRecv(const map<string, string>& input) {
        map<string, string> output;
        string ciphertext = input.at("ciphertext");
        string saltString = input.at("salt");
        ciphertext += saltString;
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)ciphertext.c_str(), ciphertext.length(), digest);
        char md5string[33];
        for (int i = 0; i < 16; i++) {
            sprintf(&md5string[i*2], "%02x", (unsigned int)digest[i]);
        }
        string md5string_str(md5string, std::strlen(md5string));
        string sign = input.at("sign");

        if(sign==md5string_str){
            ciphertext = input.at("ciphertext");
            DES_key_schedule desKey;
            DES_set_key((const_DES_cblock*)key.c_str(), &desKey);
            string plaintext(ciphertext.length(), '\0');
            DES_ncbc_encrypt((const unsigned char*)ciphertext.c_str(), (unsigned char*)plaintext.data(), ciphertext.length(), &desKey, (DES_cblock*)iv.c_str(), DES_DECRYPT);

            istringstream iss(plaintext);
            string pairString;
            while (getline(iss, pairString)) {
                istringstream pairIss(pairString);
                string key, value;
                getline(pairIss, key, '=');
                getline(pairIss, value);
                output[key] = value;
              }
        }else{
        
        }

        return output;
    }
};
