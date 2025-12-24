#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <json/json.h>
#include "../services/UserService.cc"
#include "../utils/JwtUtil.h"

using namespace drogon;

namespace im_server {
    class AuthController : public drogon::HttpController<AuthController> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(AuthController::registerUser, "/api/auth/register", Post);
        ADD_METHOD_TO(AuthController::loginUser, "/api/auth/login", Post);
        METHOD_LIST_END

        void registerUser(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
        void loginUser(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    };

    void AuthController::registerUser(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        // Get JSON data from request
        auto json = req->getJsonObject();
        
        if (!json) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "Invalid JSON data";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        std::string username = (*json)["username"].asString();
        std::string password = (*json)["password"].asString();
        std::string email = (*json)["email"].asString();

        // Validate input
        if (username.empty() || password.empty() || email.empty()) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "Username, password, and email are required";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        UserService userService;
        auto result = userService.registerUser(username, password, email);

        Json::Value ret;
        if (result.first) {
            ret["success"] = true;
            ret["message"] = "Registration successful";
            ret["user_id"] = result.second;
        } else {
            ret["success"] = false;
            ret["message"] = result.second;
        }

        auto resp = HttpResponse::newHttpJsonResponse(ret);
        if (ret["success"].asBool()) {
            resp->setStatusCode(HttpStatusCode::k201Created);
        } else {
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
        }

        callback(resp);
    }

    void AuthController::loginUser(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        // Get JSON data from request
        auto json = req->getJsonObject();
        
        if (!json) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "Invalid JSON data";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        std::string username = (*json)["username"].asString();
        std::string password = (*json)["password"].asString();

        // Validate input
        if (username.empty() || password.empty()) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "Username and password are required";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        UserService userService;
        auto result = userService.loginUser(username, password);

        Json::Value ret;
        if (result.first) {
            // Generate JWT token
            std::string token = JwtUtil::generateToken(result.second);
            ret["success"] = true;
            ret["message"] = "Login successful";
            ret["token"] = token;
            ret["user_id"] = result.second;
        } else {
            ret["success"] = false;
            ret["message"] = result.second;
        }

        auto resp = HttpResponse::newHttpJsonResponse(ret);
        if (ret["success"].asBool()) {
            resp->setStatusCode(HttpStatusCode::k200OK);
        } else {
            resp->setStatusCode(HttpStatusCode::k401Unauthorized);
        }

        callback(resp);
    }
}