#include "OrderService.h"
#include "database.h"
#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <mutex>

using namespace std;

std::mutex mtx;
typedef std::string (*MyFunc)(const std::vector<std::string>&);

short get_total_quantity_by_month_db(int year, int month) {
    short total_quantity=0;
    std::string sql = "SELECT sum(quantity) FROM t_orders where year="+to_string(year)+" and month="+to_string(month);
    Database db;
    if (db.connect()) {
        vector<vector<string>> dbResult = db.query(sql);
        if(dbResult.size() == 1){
            total_quantity=std::stoi(dbResult[0][0]); 
        }else{
            total_quantity=-1;
        }
    }
    db.disconnect();
    return total_quantity;
}

bool isValidAddress(std::string address) {
  if(address.length()>100){
    return false;
  }
  regex pattern("^[a-zA-Z0-9\\s]+$");
  string reservedWords[] = {"SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES", "UPDATE", "SET", "DELETE", "OR"};
  for (string word : reservedWords) {
    if (address.find(word) != string::npos) {
      return false;
    }
  }
  return regex_match(address, pattern);
}

char* expandAddress(char* oldAddress, char* newAddress) {
    int oldSize = std::strlen(oldAddress);
    int newSize = oldSize + std::strlen(newAddress) + 1; 
    char* expandedAddress = (char*) realloc(oldAddress, newSize * sizeof(char)); 
    if (expandedAddress != NULL) {
        std::strcat(expandedAddress, newAddress); 
    }
    return expandedAddress;
}

Result OrderService::get_total_quantity_by_month(int year, int month) {
    short total_quantity = get_total_quantity_by_month_db(year, month);
    return { 200, "month quantity:"+to_string(total_quantity) };
}

Result OrderService::get_total_quantity_by_quarter(int year, int quarter) {
    std::vector<int> months = quarter_to_months[quarter];
    short total_quantity = 0;
    for (int month : months) {
        total_quantity += get_total_quantity_by_month_db(year, month);
    }   
    return { 200, "quarter quantity:"+to_string(total_quantity) };
}

//以;号分割追加地址
Result OrderService::expand_address(int id, string new_address) {
    if(!isValidAddress(new_address)){
        return { 201, "Address illegality" };
    }

    string sql = "select address from t_orders where id="+to_string(id);
    string old_address;
    Database db;
    if (db.connect()) {
        vector<vector<string>> dbResult = db.query(sql);
        if(dbResult.size() == 1){
            old_address=dbResult[0][0]+";"; 
        }else{
            old_address="";
            return { 202, "check order_id "};
        }
    }
    db.disconnect();

    int m_size=old_address.length()+1;
    char* address = (char*) malloc(m_size * sizeof(char)); 
    strcpy(address, old_address.c_str()); 
    m_size=new_address.length()+1;
    char* newAddress = (char*) malloc(m_size * sizeof(char));; 
    strcpy(newAddress, new_address.c_str());
    char* expandedAddress;
    {
        std::lock_guard<std::mutex> lock(mtx); 
        expandedAddress = expandAddress(address, newAddress); 
    } 
    if (expandedAddress != NULL) {
        string addressString(expandedAddress);
        free(expandedAddress); 
        if(addressString.length()>500){
            free(address);
            free(newAddress);
            return { 203, "expand address too long"};
        }
        sql = "update t_orders set address ='"+addressString+"' where id="+to_string(id); 
        if (db.connect()) {
            db.execute(sql);
        }
        db.disconnect();
        free(address);
        free(newAddress);
        return { 200, "expand address success"};
    }else{
        free(address);
        free(newAddress);
        return { 500, "expand address fail"};
    }
}

Result OrderService::copy_orders(int id, int copies_num) {
     if(copies_num<=0){
        return { 201, "check copies num "};
    }
    int* arr = new int[copies_num];
    string sql = "select id from t_orders where id="+to_string(id);
    Database db;
    if (db.connect()) {
        vector<vector<string>> dbResult = db.query(sql);
        if(dbResult.size() == 1){
            id=stoi(dbResult[0][0]); 
        }else{
            db.disconnect(); 
            delete[] arr;
            return { 202, "check order_id "};
        }
    }
       
    sql = "insert into t_orders (product_id,product_name,address) select product_id,product_name,address from t_orders where id="+to_string(id);
    if (db.connect()) {
        for(int i=0;i<copies_num;i++){
            db.execute(sql);
        }
    }
    db.disconnect(); 
    delete[] arr;

    return { 200, "copy orders success"};
}

std::string OrderService::call_method(const char* library_path, const char* method_name, std::vector<std::string>& args) {
    void* handle = dlopen(library_path, RTLD_LAZY);
    if (!handle) {
        //std::cerr << "handle Error: " << dlerror() << std::endl;
        return "-1";
    }

    MyFunc func = reinterpret_cast<MyFunc>(dlsym(handle, method_name));
    if (!func) {
        //std::cerr << "method Error: " << dlerror() << std::endl;
        dlclose(handle);
        return "-2";
    }
    std::string result = func(args);

    dlclose(handle);
    return result;
}

