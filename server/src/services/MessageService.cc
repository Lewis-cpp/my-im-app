#include "Message.h"
#include "../utils/TimeUtil.h"
#include <string>
#include <vector>
#include <drogon/orm/DbClient.h>

using namespace drogon::orm;

namespace im_server {
    class MessageService {
    public:
        bool saveMessage(int64_t senderId, int64_t receiverId, 
                        const std::string& content, const std::string& messageType);
        std::vector<Message> getMessages(int64_t userId, int64_t otherUserId, int limit = 50);
        bool updateMessageAsRead(const std::string& messageId, int64_t userId);
        Message getMessageById(const std::string& messageId);
        std::vector<Message> getUnreadMessages(int64_t userId);

    private:
        DbClientPtr dbClient = DbClient::newMysqlClient("host=localhost port=3306 user=root password=password dbname=im_db", 4);
    };

    bool MessageService::saveMessage(int64_t senderId, int64_t receiverId, 
                                    const std::string& content, const std::string& messageType) {
        try {
            std::string timestamp = TimeUtil::getCurrentTimestamp();

            auto result = dbClient->execSqlSync(
                "INSERT INTO messages (sender_id, receiver_id, content, message_type, timestamp) VALUES ($1, $2, $3, $4, $5)",
                senderId, receiverId, content, messageType, timestamp
            );

            return true;
        } catch (const std::exception& e) {
            LOG_ERROR << "Error saving message: " << e.what();
            return false;
        }
    }

    std::vector<Message> MessageService::getMessages(int64_t userId, int64_t otherUserId, int limit) {
        std::vector<Message> messages;
        
        try {
            // Get messages between these two users
            auto result = dbClient->execSqlSync(
                R"(SELECT id, sender_id, receiver_id, content, message_type, timestamp, is_read 
                   FROM messages 
                   WHERE (sender_id = $1 AND receiver_id = $2) OR (sender_id = $2 AND receiver_id = $1) 
                   ORDER BY timestamp DESC LIMIT $3)",
                userId, otherUserId, limit
            );

            for (const auto& row : result) {
                Message msg(
                    row["id"].as<int64_t>(),
                    row["sender_id"].as<int64_t>(),
                    row["receiver_id"].as<int64_t>(),
                    row["content"].as<std::string>(),
                    row["message_type"].as<std::string>(),
                    row["timestamp"].as<std::string>(),
                    row["is_read"].as<bool>()
                );
                messages.push_back(msg);
            }

            // Reverse to get chronological order (oldest first)
            std::reverse(messages.begin(), messages.end());
            
        } catch (const std::exception& e) {
            LOG_ERROR << "Error getting messages: " << e.what();
        }

        return messages;
    }

    bool MessageService::updateMessageAsRead(const std::string& messageId, int64_t userId) {
        try {
            auto result = dbClient->execSqlSync(
                "UPDATE messages SET is_read = true WHERE id = $1 AND receiver_id = $2",
                std::stoll(messageId), userId
            );

            return result.affectedRows() > 0;
        } catch (const std::exception& e) {
            LOG_ERROR << "Error updating message as read: " << e.what();
            return false;
        }
    }

    Message MessageService::getMessageById(const std::string& messageId) {
        try {
            auto result = dbClient->execSqlSync(
                "SELECT id, sender_id, receiver_id, content, message_type, timestamp, is_read FROM messages WHERE id = $1",
                std::stoll(messageId)
            );

            if (result.size() > 0) {
                const auto& row = result[0];
                return Message(
                    row["id"].as<int64_t>(),
                    row["sender_id"].as<int64_t>(),
                    row["receiver_id"].as<int64_t>(),
                    row["content"].as<std::string>(),
                    row["message_type"].as<std::string>(),
                    row["timestamp"].as<std::string>(),
                    row["is_read"].as<bool>()
                );
            }
        } catch (const std::exception& e) {
            LOG_ERROR << "Error getting message by ID: " << e.what();
        }

        return Message(); // Return empty message if not found
    }

    std::vector<Message> MessageService::getUnreadMessages(int64_t userId) {
        std::vector<Message> messages;
        
        try {
            auto result = dbClient->execSqlSync(
                "SELECT id, sender_id, receiver_id, content, message_type, timestamp, is_read FROM messages WHERE receiver_id = $1 AND is_read = false ORDER BY timestamp ASC",
                userId
            );

            for (const auto& row : result) {
                Message msg(
                    row["id"].as<int64_t>(),
                    row["sender_id"].as<int64_t>(),
                    row["receiver_id"].as<int64_t>(),
                    row["content"].as<std::string>(),
                    row["message_type"].as<std::string>(),
                    row["timestamp"].as<std::string>(),
                    row["is_read"].as<bool>()
                );
                messages.push_back(msg);
            }
        } catch (const std::exception& e) {
            LOG_ERROR << "Error getting unread messages: " << e.what();
        }

        return messages;
    }
}