#pragma once

#include <string>
#include <drogon/orm/DbClient.h>

using namespace drogon::orm;

namespace im_server {
    struct User {
        int64_t id;
        std::string username;
        std::string email;
        std::string password_hash;
        std::string created_at;
        std::string updated_at;
        bool is_active;

        User() : id(0), is_active(true) {}

        User(int64_t id, const std::string& username, const std::string& email, 
             const std::string& password_hash, const std::string& created_at, 
             const std::string& updated_at, bool is_active = true)
            : id(id), username(username), email(email), password_hash(password_hash),
              created_at(created_at), updated_at(updated_at), is_active(is_active) {}
    };
}