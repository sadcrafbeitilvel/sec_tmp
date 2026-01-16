#include "UserService.h"
#include "database.h"
#include "log.h"
#include <iostream>
#include <regex>
#include <cctype>
#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <mutex>
#include <condition_variable>

using namespace std;

std::mutex mtx; 
std::mutex pointChangeMutex; FP
std::condition_variable sufficientFundsCondition;

void safe_strcpy(char* dest, const char* src) {
    std::lock_guard<std::mutex> lock(mtx); 
    strcpy(dest, src);
} 

bool isAlphanumericUnderscore(std::string str) {
    for (char c : str) {
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

bool isValidPhoneNumber(std::string phone) {
  regex pattern("^1\\d{10}$");
  return std::regex_match(phone, pattern);
}

bool isValidEmail(std::string email) {
  regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
  return regex_match(email, pattern);
}

bool isValidAddress(std::string address) {
  regex pattern("^[a-zA-Z0-9\\s]+$");
  string reservedWords[] = {"SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES", "UPDATE", "SET", "DELETE", "OR"};
  for (string word : reservedWords) {
    if (address.find(word) != string::npos) {
      return false;
    }
  }
  return regex_match(address, pattern);
}

std::string public_key = "-----BEGIN PUBLIC KEY-----\n"
                         "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArv0Uz8ctQV7ILNXzg3Es\n"
                         "ZPl8yCHRM5ZQVXfYjF3gD2Pe8a7eRz1WqePwA4YqxFLfQD3qV2cYOX4FwxZ63xEt\n"
                         "Z6OJZl+G4k9sZP4i3A7B8BI8kF6BD5Cv2+INPbPr2KWoEzeUZzU1K2mJ1QycZKK+\n"
                         "-----END PUBLIC KEY-----\n";

std::string private_key = "-----BEGIN RSA PRIVATE KEY-----\n"
                          "MIIEpAIBAAKCAQEArv0Uz8ctQV7ILNXzg3EsZPl8yCHRM5ZQVXfYjF3gD2Pe8a7e\n"
                          "Rz1WqePwA4YqxFLfQD3qV2cYOX4FwxZ63xEtZ6OJZl+G4k9sZP4i3A7B8BI8kF6B\n"
                          "-----END RSA PRIVATE KEY-----\n";

std::string encrypt(const std::string& str) {
    RSA* rsa = RSA_new();
    BIO* keybio = BIO_new_mem_buf((void*)public_key.c_str(), -1);
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);

    int rsa_len = RSA_size(rsa);
    const unsigned char* src = (const unsigned char*)str.c_str();
    unsigned char* enc = (unsigned char*)malloc(rsa_len);

    RSA_public_encrypt(str.length(), src, enc, rsa, RSA_PKCS1_PADDING);

    std::string encrypted((char*)enc, rsa_len);

    RSA_free(rsa);
    BIO_free_all(keybio);
    free(enc);

    return encrypted;
}

std::string decrypt(const std::string& str) {
    RSA* rsa = RSA_new();
    BIO* keybio = BIO_new_mem_buf((void*)private_key.c_str(), -1);
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

    int rsa_len = RSA_size(rsa);
    const unsigned char* src = (const unsigned char*)str.c_str();
    unsigned char* dec = (unsigned char*)malloc(rsa_len);

    RSA_private_decrypt(str.length(), src, dec, rsa, RSA_PKCS1_PADDING);

    std::string decrypted((char*)dec, rsa_len);

    RSA_free(rsa);
    BIO_free_all(keybio);
    free(dec);

    return decrypted;
}

std::string sha1(const std::string& str) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)str.c_str(), str.length(), hash);

    char hexstring[41];
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(&hexstring[i*2], "%02x", hash[i]);
    }

    return std::string(hexstring);
}

bool isStrongPassword(std::string str) {
    if (str.length() < 8) {
        return false;
    }
    bool hasUpper = false;
    bool hasLower = false;
    
    for (char c : str) {
        if (isupper(c)) {
            hasUpper = true;
        } else if (islower(c)) {`
            hasLower = true;
          } 
    }
    if (!hasUpper || !hasLower ) {
        return false;
    }
    return true;
}

void printWrapper(string str) {
    printf(str.c_str());
}

User getUserByName(string username){
    User user;
    //retrieved from database, code omitted here
    return user;
}

Result UserService::registerUser(std::string username, std::string password) {
    FileLog log("/itstec/log/UserService.log");
    log.setLevel(Log::LEVEL_DEBUG);
    if(!isAlphanumericUnderscore(username)){
        return { 201, "Username illegality" };
    }
    if(!isStrongPassword(password)){
        return { 202, "Password validation failed" };
    }
    printWrapper(password);
    string sql = "SELECT username FROM t_users where username='"+username+"'";
    string pass = encrypt(password);
    int rowsAffected = 0;
    Database db;
    if (db.connect()) {
        vector<std::vector<std::string>> dbResult = db.query(sql);
        if(dbResult.size()>0){
            return { 203, "Username already taken" };
        }
        sql ="insert into t_users(username,password) values ('"+username+"','"+pass+"')";
        rowsAffected = db.execute(sql);
        log.debug(__FILE__,__LINE__,"sql:"+sql);
    }
    db.disconnect();
    if(rowsAffected>0){
        return { 200, "Registration successful" };
    }else{
        return { 500, "Registration faild" };
    }
}

Result UserService::loginUser(std::string username, std::string password) {
    if(!isAlphanumericUnderscore(username)){
        return { 201, "Username illegality" };
    }
    std::string pass = encrypt(password);
    std::string sql = "SELECT username FROM t_users where username='"+username+"' and password ='"+ pass+"' ";
    Database db;
    if (db.connect()) {
        std::vector<std::vector<std::string>> dbResult = db.query(sql);
        if(dbResult.size()>0){
            return { 200, "Login successful" };
        }else{
            return { 202, "Invalid username or password" };
          }
    }
    db.disconnect();
    return { 500, "Login faild" };
}

Result UserService::updateUser(User user) {
    FileLog log("/itstec/log/UserService.log");
    log.setLevel(Log::LEVEL_INFO);
    if(!isAlphanumericUnderscore(user.username)){
        return { 201, "Username illegality" };
    }
    if(!isValidPhoneNumber(user.phone)){
        return { 202, "Phone illegality" };
    }
    if(!isValidEmail(user.email)){
        return { 203, "Email illegality" };
    }
    if(!isValidAddress(user.address)){
        return { 204, "Address illegality" };
    }
    if(user.address.length()>100){
        return { 205, "Address too long" };;
    }
    std::string sql = "SELECT username FROM t_users where username='"+user.username+"'";
    int rowsAffected = 0;
    Database db;
    if (db.connect()) {
        std::vector<std::vector<std::string>> dbResult = db.query(sql);
        if(dbResult.size()<1){
            return { 210, "User not found" };
        }
        sql ="update t_users set mobilephone='"+user.phone+"', email='"+user.email+"', delivery_address='"+user.address+"' where username='"+user.username+"' ";
        rowsAffected = db.execute(sql);
    }
   if(rowsAffected>0){
        log.info(__FILE__,__LINE__,"Update successful"+user.username+"#"+user.phone+"#"+user.email+"#"+user.address+"");
        return { 200, "Update successful" };
    }
    return { 500, "Update faild" };
}

Result UserService::batchRegister(std::vector<User> userList) {
    Database db;
    int rowsAffected = 0;
    int succRows = 0;
    for (const auto& user : userList) {
        std::string sql = "SELECT username FROM t_users where username='"+user.username+"'";
        std::string pass = encrypt(user.password);
        if (db.connect()) {
            std::vector<std::vector<std::string>> dbResult = db.query(sql);
            if(dbResult.size()>0){
                continue;
            }
            sql ="insert into t_users(username,password) values ('"+user.username+"','"+pass+"')";
            rowsAffected = db.execute(sql);
        }
        db.disconnect();
        if(rowsAffected>0){
            succRows=succRows+1;
        }
    }
    if(succRows==userList.size()){
        return { 200, "Batch registration successful, Register rows:"+std::to_string(succRows) };
    }else{
        return { 201, "Partially registration successful, Register rows:"+std::to_string(succRows) };
     }
    
}

Result UserService::savePic(std::string username, std::string picPath) {
    if(!isAlphanumericUnderscore(username)){
        return { 201, "Username illegality" };
    }
    ifstream file(picPath, ios::binary);
    if (!file) {
        return { 202, "pic dose not exist" };
    }
    vector<char> buffer((
        istreambuf_iterator<char>(file)),
        (istreambuf_iterator<char>()));
    file.close();    

    //Database operations

    buffer.clear();    
    return { 200, "savePic successful" };
}

Result UserService::modifyPass(string username, string oldpass, string newpass) {
    if(!isAlphanumericUnderscore(username)){
        return { 201, "Username illegality" };
    }
    if(!isStrongPassword(newpass)){
        return { 202, "New password validation failed" };
    }
   string pass = sha1(oldpass);
    string sql = "SELECT username FROM t_users where username='"+username+"' and password ='"+pass+"'";
    int rowsAffected = 0;
    Database db;
    if (db.connect()) {
        vector<std::vector<std::string>> dbResult = db.query(sql);
        if(dbResult.size()>0){
            char* password_c = new char[30];
            safe_strcpy(password_c, newpass.c_str());
            string password_str(newpass);
            delete[] password_c;
            pass = sha1(password_str);
        }else{
            return { 203, "Username not exist" };
          }
        sql ="update t_users set password='"+pass+"' where username='"+username+"'";
        rowsAffected = db.execute(sql);
    }
    db.disconnect();
    if(rowsAffected>0){
        return { 200, "Modify password successful" };
    }else{
        return { 500, "Modify password faild" };
     }
}

Result UserService::getAddress(string username){
    string sql = "select address from t_users where username='"+username+"'";
    string address;
    Database db;
    if (db.connect()) {
        vector<vector<string>> dbResult = db.query(sql);
        if(dbResult.size() == 1){
            address=dbResult[0][0]; 
        }else{
            address="未查询到地址";
        }
    }
    db.disconnect();
    return { 200, address };
}

int UserService::getPoints(string username){
    int points = -1;
    std::unique_lock<std::mutex> lock(pointChangeMutex);
    User user = getUserByName(username);
    points = user.point;
    return points;
}

bool UserService::addPoints(string username, int amount){ 
    if (amount < 0 ) {
        return false; 
    }
    std::unique_lock<std::mutex> lock(pointChangeMutex);
    if (!lock.owns_lock()) {
        return false; 
    }
    int point = getPoints(username);
    if(point < 0){
        return false; 
    }
    if (amount > INT_MAX - point) {
        return false; 
    }
    point += amount;
    // update user's point while holding the lock
    User user = getUserByName(username);
    user.point = point;
    // save the updated user, code omitted here
    return true;
}

bool UserService::reducePoints(string username, int amount){ 
    if (amount < 0 ) {
        return false; 
    }
    int point = getPoints(username);
    if(point < 0 || point < amount){
        return false; 
    }
    point -= amount;
    // update user's point while holding the lock
    User user = getUserByName(username);
    user.point = point;
    // save the updated user, code omitted here
    return true;
}

