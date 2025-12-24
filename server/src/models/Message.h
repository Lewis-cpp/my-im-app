#pragma once

#include <string>
#include <drogon/orm/DbClient.h>

using namespace drogon::orm;

namespace im_server {
    struct Message {
        int64_t id;
        int64_t sender_id;
        int64_t receiver_id;
        std::string content;
        std::string message_type; // text, image, file, etc.
        std::string timestamp;
        bool is_read;
        std::string file_path; // For file messages

        Message() : id(0), sender_id(0), receiver_id(0), is_read(false) {}

        Message(int64_t id, int64_t sender_id, int64_t receiver_id, const std::string& content,
                const std::string& message_type, const std::string& timestamp, 
                bool is_read = false, const std::string& file_path = "")
            : id(id), sender_id(sender_id), receiver_id(receiver_id), content(content),
              message_type(message_type), timestamp(timestamp), is_read(is_read), 
              file_path(file_path) {}
    };
}