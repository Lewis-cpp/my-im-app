cmake_minimum_required(VERSION 3.5)
project(im_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 寻找核心依赖包
find_package(Drogon REQUIRED)
find_package(Jsoncpp REQUIRED) # 必须显式寻找 jsoncpp

# 2. 自动包含 src 目录，这样代码里可以用 #include "models/User.h"
include_directories(${PROJECT_SOURCE_DIR}/src)

# 3. 核心修复：添加 jsoncpp 的头文件路径，解决 "json/json.h" 找不到的问题
include_directories(${JSONCPP_INCLUDE_DIRS})

# 4. 定义源文件 (建议按模块组织，方便维护)
set(SOURCES
    src/main.cc
    src/controllers/AuthController.cc
    src/controllers/ChatController.cc
    src/controllers/FileController.cc
    src/services/UserService.cc
    src/services/MessageService.cc
)

# 5. 添加执行文件
add_executable(im_server ${SOURCES})

# 6. 链接库
# 注意：链接 Drogon::Drogon 通常会自动处理大部分依赖
target_link_libraries(im_server PRIVATE 
    Drogon::Drogon
    ${JSONCPP_LIBRARIES}
)

# 7. 设置输出目录
set_target_properties(im_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 8. 拷贝配置文件到 build 目录
if(EXISTS "${PROJECT_SOURCE_DIR}/config.json")
    configure_file(config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)
endif()