cmake_minimum_required(VERSION 3.5)
project(im_server)

set(CMAKE_CXX_STANDARD 17)

# Find Drogon
find_package(Drogon REQUIRED)

# Add executable
add_executable(im_server
    src/main.cc
    src/controllers/AuthController.cc
    src/controllers/ChatController.cc
    src/controllers/FileController.cc
    src/models/User.h
    src/models/Message.h
    src/services/UserService.cc
    src/services/MessageService.cc
    src/utils/TimeUtil.h
    src/utils/JwtUtil.h
)

# Link Drogon
target_link_libraries(im_server PRIVATE Drogon::Drogon)

# Set output directory to build
set_target_properties(im_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy config.json to build directory
configure_file(config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)