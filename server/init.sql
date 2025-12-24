-- IM Database Initialization Script
-- Creates tables for User and Message entities with proper constraints

-- Create database if it doesn't exist
CREATE DATABASE IF NOT EXISTS im_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE im_db;

-- Create users table
CREATE TABLE users (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT TRUE,
    INDEX idx_username (username),
    INDEX idx_email (email),
    INDEX idx_is_active (is_active)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Create messages table
CREATE TABLE messages (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    sender_id BIGINT UNSIGNED NOT NULL,
    receiver_id BIGINT UNSIGNED NOT NULL,
    content TEXT,
    message_type ENUM('text', 'image', 'file') DEFAULT 'text',
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_read BOOLEAN DEFAULT FALSE,
    file_path VARCHAR(500) NULL,
    INDEX idx_sender (sender_id),
    INDEX idx_receiver (receiver_id),
    INDEX idx_timestamp (timestamp),
    INDEX idx_is_read (is_read),
    FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (receiver_id) REFERENCES users(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Insert test data
-- Add 2 test users
INSERT INTO users (username, email, password_hash, created_at, updated_at, is_active) VALUES
('alice', 'alice@example.com', 'hashed_password_alice', NOW(), NOW(), TRUE),
('bob', 'bob@example.com', 'hashed_password_bob', NOW(), NOW(), TRUE);

-- Add 1 test message
INSERT INTO messages (sender_id, receiver_id, content, message_type, timestamp, is_read, file_path) VALUES
(1, 2, 'Hello Bob! This is a test message from Alice.', 'text', NOW(), FALSE, NULL);

-- Create composite index for efficient message retrieval between users
CREATE INDEX idx_messages_sender_receiver ON messages (sender_id, receiver_id);