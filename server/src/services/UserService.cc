#include "UserService.h"
#include <string>
#include <tuple>
#include <regex>

using namespace drogon::orm;

namespace im_server {

    std::pair<bool, std::string> UserService::registerUser(const std::string& username, 
                                                          const std::string& password, 
                                                          const std::string& email) {
        // Validate input
        if (username.empty() || password.empty() || email.empty()) {
            return {false, "Username, password, and email are required"};
        }

        if (!validateEmail(email)) {
            return {false, "Invalid email format"};
        }

        if (username.length() < 3 || username.length() > 30) {
            return {false, "Username must be between 3 and 30 characters"};
        }

        if (password.length() < 6) {
            return {false, "Password must be at least 6 characters"};
        }

        try {
            // Check if username or email already exists
            auto result = dbClient->execSqlSync(
                "SELECT id FROM users WHERE username = $1 OR email = $2", 
                username, email
            );

            if (result.size() > 0) {
                // Check if it's a username conflict or email conflict
                auto existingUser = dbClient->execSqlSync(
                    "SELECT id FROM users WHERE username = $1", username
                );
                
                if (existingUser.size() > 0) {
                    return {false, "Username already exists"};
                } else {
                    return {false, "Email already exists"};
                }
            }

            // Hash the password
            std::string hashedPassword = hashPassword(password);
            std::string createdAt = TimeUtil::getCurrentTimestamp();
            std::string updatedAt = createdAt;

            // Insert the new user
            auto insertResult = dbClient->execSqlSync(
                "INSERT INTO users (username, email, password_hash, created_at, updated_at) VALUES ($1, $2, $3, $4, $5) RETURNING id",
                username, email, hashedPassword, createdAt, updatedAt
            );

            if (insertResult.size() > 0) {
                auto userId = insertResult[0]["id"].as<int64_t>();
                return {true, std::to_string(userId)};
            } else {
                return {false, "Failed to register user"};
            }
        } catch (const std::exception& e) {
            LOG_ERROR << "Error registering user: " << e.what();
            return {false, "Database error: " + std::string(e.what())};
        }
    }

    std::pair<bool, std::string> UserService::loginUser(const std::string& username, 
                                                       const std::string& password) {
        if (username.empty() || password.empty()) {
            return {false, "Username and password are required"};
        }

        try {
            // Find user by username
            auto result = dbClient->execSqlSync(
                "SELECT id, password_hash FROM users WHERE username = $1 AND is_active = true", 
                username
            );

            if (result.size() == 0) {
                return {false, "Invalid username or password"};
            }

            auto userId = result[0]["id"].as<int64_t>();
            auto storedHash = result[0]["password_hash"].as<std::string>();

            // Verify password
            if (verifyPassword(password, storedHash)) {
                return {true, std::to_string(userId)};
            } else {
                return {false, "Invalid username or password"};
            }
        } catch (const std::exception& e) {
            LOG_ERROR << "Error logging in user: " << e.what();
            return {false, "Database error: " + std::string(e.what())};
        }
    }

    bool UserService::validateEmail(const std::string& email) {
        const std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        return std::regex_match(email, pattern);
    }

    std::string UserService::hashPassword(const std::string& password) {
        // In a real application, you would use a secure hashing algorithm like bcrypt
        // For this example, we'll use a simple hash (not suitable for production)
        return "hashed_" + password; // Placeholder for actual hashing
    }

    bool UserService::verifyPassword(const std::string& password, const std::string& hash) {
        // In a real application, you would use a secure password verification function
        return hashPassword(password) == hash;
    }
}