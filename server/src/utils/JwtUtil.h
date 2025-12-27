#pragma once

#include <string>
#include <jwt-cpp/jwt.h>
#include <memory>

namespace im_server
{
    class JwtUtil
    {
    public:
        static std::string generateToken(const std::string &userId);
        static std::string verifyToken(const std::string &token);
        static std::string decodeToken(const std::string &token);

    private:
        static const std::string SECRET_KEY;
    };

    // In a real application, this secret should be stored securely (e.g., environment variable)
    const std::string JwtUtil::SECRET_KEY = "your-super-secret-key-change-in-production";

    std::string JwtUtil::generateToken(const std::string &userId)
    {
        auto token = jwt::create()
                         .set_type("JWT")
                         .set_issued_at(std::chrono::system_clock::now())
                         .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{3600 * 24}) // 24 hours
                         .set_payload_claim("user_id", jwt::claim(userId))
                         .sign(jwt::algorithm::hs256{SECRET_KEY});

        return token;
    }

    std::string JwtUtil::verifyToken(const std::string &token)
    {
        try
        {
            auto decoded = jwt::decode(token);

            // Verify signature
            auto verifier = jwt::verify()
                                .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
                                .with_issuer("im_server");

            verifier.verify(decoded);

            // Check if token is expired
            if (decoded.has_expires_at() && decoded.get_expires_at() < std::chrono::system_clock::now())
            {
                return "";
            }

            // Get user_id from payload
            if (decoded.has_payload_claim("user_id"))
            {
                return decoded.get_payload_claim("user_id").as_string();
            }

            return "";
        }
        catch (const std::exception &e)
        {
            LOG_ERROR << "JWT verification failed: " << e.what();
            return "";
        }
    }

    std::string JwtUtil::decodeToken(const std::string &token)
    {
        try
        {
            auto decoded = jwt::decode(token);

            if (decoded.has_payload_claim("user_id"))
            {
                return decoded.get_payload_claim("user_id").as_string();
            }

            return "";
        }
        catch (const std::exception &e)
        {
            LOG_ERROR << "JWT decoding failed: " << e.what();
            return "";
        }
    }
}