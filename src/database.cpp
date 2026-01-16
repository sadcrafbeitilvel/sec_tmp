#include "database.h"

Database::Database()
    : connection_(nullptr)
{
}

Database::~Database()
{
    disconnect();
}

bool Database::connect()
{
    connection_ = mysql_init(nullptr);

    if (!connection_) {
        //std::cerr << "Failed to initialize MySQL connection" << std::endl;
        return false;
    }

    if (!mysql_real_connect(connection_, "101.101.101.101", "itstec", "WtWfjwDb", "itstec", 0, nullptr, 0)) {
        std::cerr << "Failed to connect to database: " << mysql_error(connection_) << std::endl;
        return false;
    }

    return true;
}

void Database::disconnect()
{
    if (connection_) {
        mysql_close(connection_);
        connection_ = nullptr;
    }
}

std::vector<std::vector<std::string>> Database::query(std::string sql)
{
    std::vector<std::vector<std::string>> result;

    if (mysql_query(connection_, sql.c_str())) {
        //std::cerr << "Failed to execute query: " << mysql_error(connection_) << std::endl;
        return result;
    }

    MYSQL_RES* queryResult = mysql_store_result(connection_);

    if (!queryResult) {
        //std::cerr << "Failed to store query result: " << mysql_error(connection_) << std::endl;
        return result;
    }

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(queryResult))) {
        std::vector<std::string> rowValues;

        for (unsigned int i = 0; i < mysql_num_fields(queryResult); i++) {
            rowValues.push_back(row[i] ? row[i] : "NULL");
        }

        result.push_back(rowValues);
    }

    mysql_free_result(queryResult);

    return result;
}

int Database::execute(std::string sql)
{
    if (mysql_query(connection_, sql.c_str())) {
        //std::cerr << "Failed to execute query: " << mysql_error(connection_) << std::endl;
        return -1;
    }

    return mysql_affected_rows(connection_);
}
