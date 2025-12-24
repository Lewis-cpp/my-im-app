#pragma once

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace im_server {
    class TimeUtil {
    public:
        static std::string getCurrentTimestamp();
        static std::string formatTimestamp(std::time_t time);
        static std::time_t getCurrentTime();
        static std::string timeToString(std::time_t time);
        static std::time_t stringToTime(const std::string& str);
    };

    std::string TimeUtil::getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        return ss.str();
    }

    std::string TimeUtil::formatTimestamp(std::time_t time) {
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::time_t TimeUtil::getCurrentTime() {
        return std::time(nullptr);
    }

    std::string TimeUtil::timeToString(std::time_t time) {
        return std::to_string(time);
    }

    std::time_t TimeUtil::stringToTime(const std::string& str) {
        // Parsing a string like "2022-01-01 12:00:00" to time_t
        // This is a simplified version - a more robust implementation would be needed for production
        struct std::tm tm = {};
        int year, month, day, hour, minute, second;
        sscanf_s(str.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
        
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        
        return std::mktime(&tm);
    }
}