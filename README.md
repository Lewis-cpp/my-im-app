# MyIMProject

## 项目概览
MyIMProject 是一个即时通讯应用，包含前端 Electron + React 客户端、C++ Drogon 后端服务以及命令行测试工具。

## 项目结构
- `client/` - Electron + React 前端客户端
- `server/` - C++ Drogon 后端服务
- `cli_test/` - C++ 命令行测试工具
- `docs/` - 项目文档

## 运行指南

### 后端服务 (server/)
```bash
cd server/
mkdir build && cd build/
cmake ..
make
./server
```

### 前端客户端 (client/)
```bash
cd client/
npm install
npm start
```

### 命令行测试工具 (cli_test/)
```bash
cd cli_test/
mkdir build && cd build/
cmake ..
make
./cli_test
```

## 依赖项
- C++17 或更高版本
- CMake 3.10 或更高版本
- Node.js (前端)
- npm (前端)
- Drogon 框架 (后端)
- Electron (前端)
- React (前端)