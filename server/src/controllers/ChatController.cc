#include <drogon/WebSocketController.h>
#include <json/json.h>
#include "../services/MessageService.h"
#include "../utils/JwtUtil.h"
#include <unordered_map>
#include <mutex>

using namespace drogon;

namespace im_server
{
    class ChatController : public drogon::WebSocketController<ChatController>
    {
    public:
        // ✅ 修复核心：手动实现路由注册
        // 关键：必须使用 this->registerPath，否则在模板继承中编译器找不到该函数
        void initPathRouting() override
        {
            this->registerPath("/ws/chat");
        }

        // 处理新连接
        void handleNewConnection(const HttpRequestPtr &req,
                                 const WebSocketConnectionPtr &wsConnPtr) override;

        // 处理新消息
        void handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                              std::string &&message,
                              const WebSocketMessageType &type) override;

        // 处理连接关闭
        void handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr) override;

    private:
        struct UserSession
        {
            std::string userId;
            std::string username;
        };

        // 存储活跃连接
        static std::unordered_map<WebSocketConnectionPtr, UserSession> connections_;
        static std::mutex connections_mutex_;
    };

    // 静态成员定义
    std::unordered_map<WebSocketConnectionPtr, ChatController::UserSession> ChatController::connections_;
    std::mutex ChatController::connections_mutex_;

    // 实现：处理新连接
    void ChatController::handleNewConnection(const HttpRequestPtr &req,
                                             const WebSocketConnectionPtr &wsConnPtr)
    {
        // 1. 获取 Token
        std::string token = req->getParameter("token");
        if (token.empty())
        {
            auto authHeader = req->getHeader("Authorization");
            if (authHeader.substr(0, 7) == "Bearer ")
            {
                token = authHeader.substr(7);
            }
        }

        if (token.empty())
        {
            LOG_WARN << "Missing token";
            wsConnPtr->forceClose();
            return;
        }

        // 2. 验证 Token
        auto userId = JwtUtil::verifyToken(token);
        if (userId.empty())
        {
            LOG_WARN << "Invalid token";
            wsConnPtr->forceClose();
            return;
        }

        // 3. 记录连接
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_[wsConnPtr] = {userId, ""};
        }

        LOG_INFO << "New WebSocket connection from user: " << userId;

        // 4. 发送欢迎消息
        Json::Value response;
        response["type"] = "connected";
        response["message"] = "WebSocket connection established";
        wsConnPtr->send(Json::writeString(Json::StreamWriterBuilder(), response));
    }

    // 实现：处理消息
    void ChatController::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                                          std::string &&message,
                                          const WebSocketMessageType &type)
    {
        if (type != WebSocketMessageType::Text)
            return;

        try
        {
            Json::Value json;
            Json::CharReaderBuilder builder;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            std::string errors;

            // 解析 JSON
            bool parsingSuccessful = reader->parse(
                message.c_str(),
                message.c_str() + message.size(),
                &json,
                &errors);

            if (!parsingSuccessful)
            {
                LOG_ERROR << "Failed to parse JSON: " << errors;
                return;
            }

            // 获取当前用户信息
            UserSession userSession;
            {
                std::lock_guard<std::mutex> lock(connections_mutex_);
                auto it = connections_.find(wsConnPtr);
                if (it == connections_.end())
                    return;
                userSession = it->second;
            }

            std::string msgType = json["type"].asString();

            if (msgType == "message")
            {
                std::string toUserIdStr = json["to"].asString();
                std::string content = json["content"].asString();

                if (toUserIdStr.empty() || content.empty())
                    return;

                MessageService messageService;

                try
                {
                    // ID 类型转换
                    int64_t senderId = std::stoll(userSession.userId);
                    int64_t receiverId = std::stoll(toUserIdStr);

                    // 保存消息
                    bool saved = messageService.saveMessage(senderId, receiverId, content, "text");
                    if (!saved)
                    {
                        LOG_ERROR << "Failed to save message";
                        return;
                    }

                    // 转发消息
                    {
                        std::lock_guard<std::mutex> lock(connections_mutex_);
                        for (const auto &conn : connections_)
                        {
                            if (conn.second.userId == toUserIdStr)
                            {
                                Json::Value messageResponse;
                                messageResponse["type"] = "message";
                                messageResponse["from"] = userSession.userId;
                                messageResponse["content"] = content;
                                messageResponse["timestamp"] = TimeUtil::getCurrentTimestamp();
                                conn.first->send(Json::writeString(Json::StreamWriterBuilder(), messageResponse));
                            }
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    LOG_ERROR << "Message processing error: " << e.what();
                }
            }
            else if (msgType == "read_receipt")
            {
                std::string messageId = json["message_id"].asString();
                MessageService messageService;
                try
                {
                    int64_t userId = std::stoll(userSession.userId);
                    messageService.updateMessageAsRead(messageId, userId);
                }
                catch (...)
                {
                }
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR << "WebSocket error: " << e.what();
        }
    }

    // 实现：连接关闭
    void ChatController::handleConnectionClosed(const WebSocketConnectionPtr &wsConnPtr)
    {
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_.erase(wsConnPtr);
        }
        LOG_INFO << "WebSocket connection closed";
    }
}