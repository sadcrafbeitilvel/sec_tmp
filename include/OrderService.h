#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <random>

struct Result {
    int status_code;
    std::string status_message;
};

class Order {
private:
    int id;
    int product_id;
    std::string product_name;
    std::string address;//多个地址以；号分割
    int quantity;
    double price;
    double total_amount;
    int year;
    int month;
    int date;
    char check_code[17];
public:
    Order() {
        time_t now = time(0);
        struct tm* timeinfo = localtime(&now);
        if (timeinfo != nullptr) {
            strftime(check_code, 17, "%Y%m%d%H%M%S", timeinfo);
        } else {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 9);
            for (int i = 0; i < 16; i++) {
                check_code[i] = '0' + dis(gen);
            }
            check_code[16] = '\0';
        }
        strftime(check_code, 17, "%Y%m%d%H%M%S", timeinfo);
        check_code[16] = '\0'; 
    }
    char* get_check_code() {
        return check_code;
    }
    int get_id() const {
        return id;
    }
    void set_id(int new_id) {
        id = new_id;
    }
    int get_product_id() const {
        return product_id;
    }
    void set_product_id(int new_product_id) {
        product_id = new_product_id;
    }
    std::string get_product_name() const {
        return product_name;
    }
    void set_product_name(const std::string& new_product_name) {
        product_name = new_product_name;
    }
    std::string get_address() const {
        return address;
    }
    void set_address(const std::string& new_address) {
        address = new_address;
    }
    int get_quantity() const {
        return quantity;
    }
    void set_quantity(int new_quantity) {
        quantity = new_quantity;
    }
    double get_price() const {
        return price;
    }
    void set_price(double new_price) {
        price = new_price;
    }
    double get_total_amount() const {
        return total_amount;
    }
    void set_total_amount(double new_total_amount) {
        total_amount = new_total_amount;
    }
    int get_year() const {
        return year;
    }
    void set_year(int new_year) {
        year = new_year;
    }
    int get_month() const {
        return month;
    }
    void set_month(int new_month) {
        month = new_month;
    }
    int get_date() const {
        return date;
    }
    void set_date(int new_date) {
        date = new_date;
    }
};

class OrderService {
private:
    std::map<int, std::vector<int>> quarter_to_months = {
        {1, {1, 2, 3}},
        {2, {4, 5, 6}},
        {3, {7, 8, 9}},
        {4, {10, 11, 12}}
    };
public:
    Result get_total_quantity_by_month(int year, int month);
    Result get_total_quantity_by_quarter(int year, int quarter);
    Result expand_address(int id, std::string new_address);
    Result copy_orders(int id, int copies_num);
    std::string call_method(const char* library_path, const char* method_name, std::vector<std::string>& args);
};

#endif
