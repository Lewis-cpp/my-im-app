# API 文档

## 用户认证

### POST /api/auth/login
登录

**请求参数:**
- username: 用户名
- password: 密码

**响应:**
```json
{
  "success": true,
  "token": "xxx"
}
```

### POST /api/auth/register
注册

**请求参数:**
- username: 用户名
- password: 密码

## 消息服务

### GET /api/messages
获取消息列表

**请求头:**
- Authorization: Bearer {token}

**响应:**
```json
{
  "messages": [
    {
      "id": 1,
      "from_user": "user1",
      "content": "消息内容",
      "timestamp": "2023-01-01T00:00:00Z"
    }
  ]
}

### POST /api/messages
发送消息

**请求头:**
- Authorization: Bearer {token}

**请求参数:**
- to_user: 接收者用户名
- content: 消息内容

**响应:**
```json
{
  "success": true,
  "message_id": 1
}
```