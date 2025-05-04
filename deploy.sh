#!/bin/bash

# 部署脚本

# 安装依赖
echo "Installing dependencies..."
apt-get update
apt-get install -y python3 python3-pip python3-dev
apt-get install -y nodejs npm
apt-get install -y openjdk-11-jdk
apt-get install -y golang
apt-get install -y curl

# 创建目录结构
echo "Setting up directory structure..."
mkdir -p /var/www/klpbbs
mkdir -p /var/www/klpbbs/uploads

# 克隆代码仓库
echo "Cloning code repositories..."
git clone https://github.com/yourusername/klpbbs-python.git /var/www/klpbbs/python
git clone https://github.com/yourusername/klpbbs-java.git /var/www/klpbbs/java
git clone https://github.com/yourusername/klpbbs-cpp.git /var/www/klpbbs/cpp
git clone https://github.com/yourusername/klpbbs-javascript.git /var/www/klpbbs/javascript
git clone https://github.com/yourusername/klpbbs-go.git /var/www/klpbbs/go
git clone https://github.com/yourusername/klpbbs-c.git /var/www/klpbbs/c
git clone https://github.com/yourusername/klpbbs-ruby.git /var/www/klpbbs/ruby
git clone https://github.com/yourusername/klpbbs-typescript.git /var/www/klpbbs/typescript
git clone https://github.com/yourusername/klpbbs-rust.git /var/www/klpbbs/rust

# 安装Python依赖
echo "Installing Python dependencies..."
cd /var/www/klpbbs/python
pip3 install -r requirements.txt

# 构建Java项目
echo "Building Java project..."
cd /var/www/klpbbs/java
./mvnw clean package

# 构建C++项目
echo "Building C++ project..."
cd /var/www/klpbbs/cpp
cmake .
make

# 安装JavaScript依赖
echo "Installing JavaScript dependencies..."
cd /var/www/klpbbs/javascript
npm install

# 构建Go项目
echo "Building Go project..."
cd /var/www/klpbbs/go
go build

# 构建Rust项目
echo "Building Rust project..."
cd /var/www/klpbbs/rust
cargo build --release

# 设置环境变量
echo "Setting environment variables..."
echo "export JWT_SECRET='your_jwt_secret'" >> /etc/profile
echo "export DATABASE_URL='postgres://user:password@localhost/dbname'" >> /etc/profile

# 启动服务
echo "Starting services..."
systemctl restart nginx
systemctl restart postgresql

# 设置定时任务
echo "Setting up cron jobs..."
(crontab -l ; echo "0 0 * * * /usr/bin/python3 /var/www/klpbbs/python/cron.py") | crontab -

echo "Deployment completed successfully!"