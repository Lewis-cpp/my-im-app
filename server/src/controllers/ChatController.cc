#include <drogon/WebSocketController.h>
#include <json/json.h>
#include "../services/MessageService.h"
#include "../utils/JwtUtil.h"
#include <unordered_map>
#include <mutex>

using namespace drogon;

namespace im_server {
    class ChatController : public drogon::WebSocketController<ChatController> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(ChatController::newWebsocket, "/ws/chat", {Get});
        METHOD_LIST_END

        void newWebsocket(const HttpRequestPtr& req, 
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const WebSocketConnectionPtr& wsConnPtr) override;
        void handleNewMessage(const WebSocketConnectionPtr& wsConnPtr,
                             const std::string& message,
                             const WebSocketMessageType& type) override;
        void handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) override;

    private:
        struct UserSession {
            std::string userId;
            std::string username;
        };

        // Store active connections and user sessions
        static std::unordered_map<WebSocketConnectionPtr, UserSession> connections_;
        static std::mutex connections_mutex_;
    };

    // Static member definitions
    std::unordered_map<WebSocketConnectionPtr, ChatController::UserSession> ChatController::connections_;
    std::mutex ChatController::connections_mutex_;

    void ChatController::newWebsocket(const HttpRequestPtr& req, 
                                     std::function<void(const HttpResponsePtr&)>&& callback,
                                     const WebSocketConnectionPtr& wsConnPtr) {
        // Get JWT token from query parameters or headers
        std::string token = req->getParameter("token");
        if (token.empty()) {
            // Try to get token from Authorization header
            auto authHeader = req->getHeader("Authorization");
            if (authHeader.substr(0, 7) == "Bearer ") {
                token = authHeader.substr(7);
            }
        }

        if (token.empty()) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k401Unauthorized);
            callback(resp);
            return;
        }

        // Verify JWT token
        auto userId = JwtUtil::verifyToken(token);
        if (userId.empty()) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k401Unauthorized);
            callback(resp);
            return;
        }

        // Add connection to active connections list
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_[wsConnPtr] = {"", ""}; // We'll update this after getting user info
        }

        // Send connection confirmation
        Json::Value response;
        response["type"] = "connected";
        response["message"] = "WebSocket connection established";
        wsConnPtr->send(Json::writeString(Json::StreamWriterBuilder(), response));

        // Authenticate user and update session
        // For now, just set the userId
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_[wsConnPtr].userId = userId;
        }

        LOG_INFO << "New WebSocket connection from user: " << userId;
        callback(HttpResponse::newHttpResponse());
    }

    void ChatController::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr,
                                         const std::string& message,
                                         const WebSocketMessageType& type) {
        if (type != WebSocketMessageType::Text) {
            return; // Only handle text messages
        }

        try {
            Json::Value json;
            Json::CharReaderBuilder builder;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            std::string errors;

            bool parsingSuccessful = reader->parse(
                message.c_str(), 
                message.c_str() + message.size(), 
                &json, 
                &errors
            );

            if (!parsingSuccessful) {
                LOG_ERROR << "Failed to parse JSON message: " << errors;
                return;
            }

            // Get user session
            UserSession userSession;
            {
                std::lock_guard<std::mutex> lock(connections_mutex_);
                auto it = connections_.find(wsConnPtr);
                if (it == connections_.end()) {
                    LOG_ERROR << "Connection not found in session";
                    return;
                }
                userSession = it->second;
            }

            std::string msgType = json["type"].asString();
            
            if (msgType == "message") {
                std::string toUserId = json["to"].asString();
                std::string content = json["content"].asString();
                
                // Validate message
                if (toUserId.empty() || content.empty()) {
                    Json::Value errorResponse;
                    errorResponse["type"] = "error";
                    errorResponse["message"] = "Missing recipient or content";
                    wsConnPtr->send(Json::writeString(Json::StreamWriterBuilder(), errorResponse));
                    return;
                }

                // Save message to database
                MessageService messageService;
                bool saved = messageService.saveMessage(userSession.userId, toUserId, content, "text");
                
                if (!saved) {
                    Json::Value errorResponse;
                    errorResponse["type"] = "error";
                    errorResponse["message"] = "Failed to save message";
                    wsConnPtr->send(Json::writeString(Json::StreamWriterBuilder(), errorResponse));
                    return;
                }

                // Send message to recipient if they're connected
                {
                    std::lock_guard<std::mutex> lock(connections_mutex_);
                    for (const auto& conn : connections_) {
                        if (conn.second.userId == toUserId) {
                            Json::Value messageResponse;
                            messageResponse["type"] = "message";
                            messageResponse["from"] = userSession.userId;
                            messageResponse["content"] = content;
                            messageResponse["timestamp"] = std::to_string(time(nullptr));
                            conn.first->send(Json::writeString(Json::StreamWriterBuilder(), messageResponse));
                        }
                    }
                }

                // Confirm message sent to sender
                Json::Value confirmResponse;
                confirmResponse["type"] = "message_sent";
                confirmResponse["message_id"] = "temp_id"; // In a real app, use actual message ID from DB
                wsConnPtr->send(Json::writeString(Json::StreamWriterBuilder(), confirmResponse));
            }
            else if (msgType == "typing") {
                std::string toUserId = json["to"].asString();
                
                // Broadcast typing indicator to recipient
                {
                    std::lock_guard<std::mutex> lock(connections_mutex_);
                    for (const auto& conn : connections_) {
                        if (conn.second.userId == toUserId) {
                            Json::Value typingResponse;
                            typingResponse["type"] = "typing";
                            typingResponse["from"] = userSession.userId;
                            conn.first->send(Json::writeString(Json::StreamWriterBuilder(), typingResponse));
                        }
                    }
                }
            }
            else if (msgType == "read_receipt") {
                std::string messageId = json["message_id"].asString();
                
                // Update message as read in database
                MessageService messageService;
                messageService.updateMessageAsRead(messageId, userSession.userId);
            }
            else {
                Json::Value errorResponse;
                errorResponse["type"] = "error";
                errorResponse["message"] = "Unknown message type";
                wsConnPtr->send(Json::writeString(Json::StreamWriterBuilder(), errorResponse));
            }
        }
        catch (const std::exception& e) {
            LOG_ERROR << "Error handling WebSocket message: " << e.what();
        }
    }

    void ChatController::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) {
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_.erase(wsConnPtr);
        }
        
        LOG_INFO << "WebSocket connection closed";
    }
}