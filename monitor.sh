#!/bin/bash

# 服务器监控脚本

# 配置
CHECK_INTERVAL=60 # 检查间隔（秒）
LOG_FILE="/var/log/klpbbs_monitor.log"

# 检查服务状态
check_service() {
    local service_name=$1
    systemctl is-active --quiet $service_name
    return $?
}

# 检查端口监听
check_port() {
    local port=$1
    netstat -tuln | grep -q ":$port "
    return $?
}

# 检查磁盘空间
check_disk_space() {
    local min_free_percent=$1
    local free_percent=$(df / | tail -1 | awk '{print $5}' | sed 's/%//')
    [ $free_percent -lt $min_free_percent ]
    return $?
}

# 检查内存使用
check_memory_usage() {
    local max_usage_percent=$1
    local usage_percent=$(free | awk '/^Mem:/{printf("%.0f", ($3/$2)*100)}')
    [ $usage_percent -gt $max_usage_percent ]
    return $?
}

# 监控服务
monitor() {
    while true; do
        # 检查服务
        for service in nginx postgresql; do
            if ! check_service $service; then
                echo "$(date): Service $service is down. Attempting to restart..." >> $LOG_FILE
                systemctl restart $service
            fi
        done

        # 检查端口
        for port in 80 443 5432; do
            if ! check_port $port; then
                echo "$(date): Port $port is not listening. Checking related services..." >> $LOG_FILE
                if [ $port -eq 80 ] || [ $port -eq 443 ]; then
                    systemctl restart nginx
                elif [ $port -eq 5432 ]; then
                    systemctl restart postgresql
                fi
            fi
        done

        # 检查磁盘空间
        if check_disk_space 10; then
            echo "$(date): Disk space is low. Free space: $free_percent%" >> $LOG_FILE
            # 可以添加清理操作或发送警报
        fi

        # 检查内存使用
        if check_memory_usage 90; then
             echo "$(date): Memory usage is high. Usage: $usage_percent%" >> $LOG_FILE
            # 可以添加清理缓存或其他优化操作
        fi

        sleep $CHECK_INTERVAL
    done
}

# 启动监控
monitor