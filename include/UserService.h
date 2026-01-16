#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <string>
#include <vector>

struct Result {
    int status_code;
    std::string status_message;
};

class User {
public:
    std::string username;
    std::string password;
    std::string phone;
    std::string email;
    std::string address;
    std::string registration_time;
    int point;
};

class UserService {
private:
    std::vector<User> users;
public:
    Result registerUser(std::string username, std::string password);
    Result loginUser(std::string username, std::string password);
    Result updateUser(User user);
    Result batchRegister(std::vector<User> userList);
    Result savePic(std::string username, std::string picPath);
    Result modifyPass(std::string username, std::string oldpass, std::string newpass);
    Result getAddress(std::string username);
    bool addPoints(std::string username, int point);
    bool reducePoints(std::string username, int point);
    int getPoints(std::string username);
};

#endif
