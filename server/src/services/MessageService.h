#pragma once

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
}