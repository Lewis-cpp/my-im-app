#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/utils/Utilities.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

using namespace drogon;

namespace im_server {
    class FileController : public drogon::HttpController<FileController> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(FileController::uploadFile, "/api/file/upload", Post);
        ADD_METHOD_TO(FileController::downloadFile, "/api/file/download/{1}", Get, "im_server::JwtFilter");
        METHOD_LIST_END

        void uploadFile(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
        void downloadFile(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& fileId);

    private:
        std::string uploadDir = "uploads/";
        std::vector<std::string> allowedTypes = {".jpg", ".jpeg", ".png", ".gif", ".pdf", ".doc", ".docx", ".txt", ".zip"};
        size_t maxFileSize = 10 * 1024 * 1024; // 10MB
    };

    void FileController::uploadFile(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        // Check if the request contains files
        if (!req->hasFile()) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "No file uploaded";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        // Get uploaded files
        const auto& files = req->getFiles();
        if (files.empty()) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "No file uploaded";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        // Validate and process the first file (in a real app, you might support multiple files)
        const auto& file = files[0];
        
        // Check file size
        if (file.getFileSize() > maxFileSize) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "File size exceeds limit (10MB)";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        // Get file extension
        std::string originalName = file.getFileName();
        std::string ext = "";
        size_t extPos = originalName.find_last_of('.');
        if (extPos != std::string::npos) {
            ext = originalName.substr(extPos);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }

        // Validate file type
        if (std::find(allowedTypes.begin(), allowedTypes.end(), ext) == allowedTypes.end()) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "File type not allowed";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        // Create upload directory if it doesn't exist
        if (!std::filesystem::exists(uploadDir)) {
            std::filesystem::create_directories(uploadDir);
        }

        // Generate unique filename to avoid conflicts
        std::string uniqueName = trantor::utils::getUuid() + ext;
        std::string filePath = uploadDir + uniqueName;

        // Save the file
        bool saved = file.moveFileToDirectory(filePath);
        if (!saved) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "Failed to save file";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k500InternalServerError);
            callback(resp);
            return;
        }

        Json::Value ret;
        ret["success"] = true;
        ret["message"] = "File uploaded successfully";
        ret["file_id"] = uniqueName;  // Using filename as ID (in a real app, you'd use a DB ID)
        ret["file_name"] = originalName;
        ret["file_size"] = (Json::Value::UInt64)file.getFileSize();
        ret["file_type"] = ext;

        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(HttpStatusCode::k201Created);
        callback(resp);
    }

    void FileController::downloadFile(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& fileId) {
        // Validate file ID
        if (fileId.empty()) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "File ID is required";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(resp);
            return;
        }

        // Check if file exists
        std::string filePath = uploadDir + fileId;
        if (!std::filesystem::exists(filePath)) {
            Json::Value ret;
            ret["success"] = false;
            ret["message"] = "File not found";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(HttpStatusCode::k404NotFound);
            callback(resp);
            return;
        }

        // Serve the file
        auto resp = HttpResponse::newFileResponse(filePath);
        resp->addHeader("Content-Disposition", "attachment; filename=\"" + fileId + "\"");
        callback(resp);
    }
}