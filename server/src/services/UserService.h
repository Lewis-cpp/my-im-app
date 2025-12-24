#pragma once

#include "User.h"
#include "../utils/TimeUtil.h"
#include <string>
#include <tuple>
#include <regex>
#include <drogon/orm/DbClient.h>

using namespace drogon::orm;

namespace im_server {
    class UserService {
    public:
        std::pair<bool, std::string> registerUser(const std::string& username,
                                                  const std::string& password,
                                                  const std::string& email);
        std::pair<bool, std::string> loginUser(const std::string& username,
                                               const std::string& password);
        bool validateEmail(const std::string& email);
        std::string hashPassword(const std::string& password);
        bool verifyPassword(const std::string& password, const std::string& hash);

    private:
        DbClientPtr dbClient = DbClient::newMysqlClient("host=localhost port=3306 user=root password=password dbname=im_db", 4);
    };
}