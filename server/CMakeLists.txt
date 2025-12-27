cmake_minimum_required(VERSION 3.5)
project(im_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 寻找依赖包
find_package(Drogon REQUIRED)
find_package(Jsoncpp REQUIRED)
find_package(OpenSSL REQUIRED) # 新增：寻找 OpenSSL

# 2. 包含路径
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR}) # 新增：包含 OpenSSL 头文件

# 3. 定义源文件
set(SOURCES
    src/main.cc
    src/controllers/AuthController.cc
    src/controllers/ChatController.cc
    src/controllers/FileController.cc
    src/services/UserService.cc
    src/services/MessageService.cc
)

# 4. 生成可执行文件
add_executable(im_server ${SOURCES})

# 5. 链接库
target_link_libraries(im_server PRIVATE 
    Drogon::Drogon
    ${JSONCPP_LIBRARIES}
    OpenSSL::SSL     # 新增：链接 SSL
    OpenSSL::Crypto  # 新增：链接 Crypto
)

# 6. 配置输出
set_target_properties(im_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 7. 拷贝配置文件
if(EXISTS "${PROJECT_SOURCE_DIR}/config.json")
    configure_file(config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)
endif()