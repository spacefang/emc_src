#!/bin/sh

# 这个脚本接收6个参数: 年 月 日 时 分 秒
# 并将它们格式化为 'date -s' 命令需要的 "YYYY-MM-DD HH:mm:ss" 格式

# 健壮性检查：如果参数数量不等于6，则退出
if [ "$#" -ne 6 ]; then
    echo "Error: This script requires 6 arguments (Y M D H m S)."
    exit 1
fi

YEAR=$1
MONTH=$2
DAY=$3
HOUR=$4
MINUTE=$5
SECOND=$6

# 将参数拼接成 date 命令需要的格式
TIME_STRING="$YEAR-$MONTH-$DAY $HOUR:$MINUTE:$SECOND"

echo "Attempting to set time to: $TIME_STRING"

# 使用绝对路径执行 date 命令
/bin/date -s "$TIME_STRING"

# 检查上一条命令是否执行成功
if [ $? -eq 0 ]; then
    echo "Time set successfully."
    exit 0
else
    echo "Error: Failed to set time."
    exit 1
fi