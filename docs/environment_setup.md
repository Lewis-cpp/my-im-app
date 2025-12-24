这是一份为您 14 位组员准备的**《IM 项目全栈开发环境配置保姆级指南》**。

这份文档直接针对您当前的 **WSL 2 + Drogon + MariaDB (3307) + Electron/React** 架构编写，确保大一同学也能顺利跑通。

---

# 🚀 IM 项目开发环境配置指南 (V1.0)

亲爱的组员，欢迎加入 IM 项目组！为了保证大家开发环境统一，请严格按照以下步骤配置你的电脑。**不要跳步！不要自行修改端口！**

## 一、 基础软件安装 (Windows 侧)

在开始 Linux 配置前，请先安装以下 Windows 工具：
1. **VS Code**: [下载地址](https://code.visualstudio.com/)
2. **Git for Windows**: [下载地址](https://git-scm.com/download/win)
3. **Node.js (LTS 版本)**: [下载地址](https://nodejs.org/en) (用于前端开发)
4. **VS Code 插件** (必须安装):
   - `WSL` (Microsoft 出品)
   - `C/C++`
   - `CMake Tools`

---

## 二、 核心：WSL 2 与 Ubuntu 24.04

我们后端跑在 Linux 环境下，请确保你的电脑开启了虚拟化。

1. **启用 WSL**: 
   以管理员身份打开 PowerShell，输入：
   ```powershell
   wsl --install -d Ubuntu-24.04
   ```
   *如果已安装其他版本，请确保它是 WSL 2。重启电脑后设置好用户名和密码。*

2. **连接 VS Code**:
   打开 VS Code，点击左下角的蓝色图标 `<>`，选择 **"Connect to WSL"**。

---

## 三、 后端环境配置 (WSL 内部)

进入 WSL 终端后，**直接复制执行**以下命令。

### 1. 安装基础编译工具
```bash
sudo apt update
sudo apt install -y build-essential cmake git gdb \
    libssl-dev zlib1g-dev libjsoncpp-dev uuid-dev \
    libmariadb-dev redis-server nlohmann-json3-dev libhiredis-dev
```

### 2. 自动化配置数据库 (MariaDB 3307)
由于 3306 端口常与 Windows 冲突，我们统一使用 **3307**。请执行组长提供的修复脚本：
```bash
# 创建并运行数据库启动脚本
cat << 'EOF' > ~/start_db.sh
#!/bin/bash
sudo mkdir -p /run/mysqld
sudo chown mysql:mysql /run/mysqld
sudo pkill -9 mariadbd
# 强制以后台模式启动 3307 端口
sudo nohup sudo -u mysql /usr/sbin/mariadbd --port=3307 > /tmp/mariadb.log 2>&1 &
echo "数据库已在后台尝试启动..."
sleep 2
mariadb -u root -P 3307 -e "SELECT VERSION();"
EOF

chmod +x ~/start_db.sh
./~/start_db.sh
```

### 3. 初始化数据库权限
执行以下命令进入数据库：
```bash
sudo mariadb -u root -P 3307
```
**进入后，依次粘贴以下 SQL：**
```sql
-- 修改 root 密码为 123456
ALTER USER 'root'@'localhost' IDENTIFIED VIA mysql_native_password USING PASSWORD('123456');
CREATE USER IF NOT EXISTS 'root'@'%' IDENTIFIED BY '123456';
GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' WITH GRANT OPTION;
-- 创建项目数据库
CREATE DATABASE IF NOT EXISTS im_db;
FLUSH PRIVILEGES;
EXIT;
```

---

## 四、 编译运行后端 (Server)

1. **克隆代码**:
   ```bash
   cd ~
   git clone <项目仓库地址>
   cd my-im-app/server
   ```

2. **导入表结构**:
   ```bash
   mariadb -u root -p123456 -P 3307 im_db < init.sql
   ```

3. **编译**:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

4. **启动**:
   ```bash
   ./im_server
   ```
   *注意：如果报错连不上数据库，请检查 `config.json` 里的 port 是否为 3307。*

---

## 五、 前端环境配置 (Client)

1. **安装 Yarn**:
   在 Windows 的 CMD 窗口输入：
   ```bash
   npm install -g yarn
   ```

2. **启动前端**:
   进入 `client` 目录：
   ```bash
   yarn install
   yarn dev
   ```

---

## 六、 架构师（组长）的特别提醒 ⚠️

1. **IP 地址问题**：
   - 因为后端在 WSL (Linux)，前端在 Windows。前端代码连接后端时，**不能**写 `localhost`。
   - 请在 WSL 终端输入 `ip addr show eth0` 找到 `inet` 后的 IP（通常是 `172.x.x.x`），这就是后端的地址。

2. **数据库端口**：
   - 全组统一使用 **3307**。如果你修改了 `config.json` 或 `db.cpp`，请务必确认端口号。

3. **开发规范**：
   - 业务逻辑请写在 `src/services/` 中。
   - 接口定义请写在 `src/controllers/` 中。
   - 严禁在 Controller 里直接写复杂的 SQL 语句，请调用 Service。

---

### ✅ 验证你的环境是否 OK：
1. 后端成功运行且没有出现 `Database connection failed`。
2. 通过 `mariadb -u root -p123456 -P 3307 im_db` 能看到 `users` 表里有 `alice` 和 `bob`。
3. 前端 Vite 页面能正常打开。

**遇到问题先看文档，解决不了再联系组长！加油！**

--- 

### 组长（架构师）说明：
您可以将此文档保存为项目根目录下的 `docs/environment_setup.md`。这样组员克隆完项目后，第一件事就是阅读这个文档。这个文档规避了您之前遇到的所有坑（端口冲突、权限问题、路径问题）。