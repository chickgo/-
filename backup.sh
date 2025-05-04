#!/bin/bash

# 数据库备份脚本

# 配置
DB_HOST="localhost"
DB_NAME="klpbbs"
DB_USER="postgres"
DB_PASSWORD="your_db_password"
BACKUP_DIR="/var/backups/klpbbs"
TIMESTAMP=$(date +"%Y%m%d%H%M%S")

# 创建备份目录
mkdir -p $BACKUP_DIR

# 执行备份
PGPASSWORD=$DB_PASSWORD pg_dump -h $DB_HOST -U $DB_USER $DB_NAME > "$BACKUP_DIR/klpbbs_$TIMESTAMP.sql"

# 检查备份是否成功
if [ $? -eq 0 ]; then
    echo "Backup successful: klpbbs_$TIMESTAMP.sql"
else
    echo "Backup failed" >&2
    exit 1
fi

# 清理旧备份（保留最近7天）
find $BACKUP_DIR -type f -name "klpbbs_*.sql" -mtime +7 -exec rm {} \;