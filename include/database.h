#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
#include <string>
#include <vector>
#include <iostream>

class Database
{
public:
    Database();
    ~Database();

    bool connect();
    void disconnect();
    std::vector<std::vector<std::string>> query(std::string sql);
    int execute(std::string sql);

private:
    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
    MYSQL* connection_;
};

#endif
