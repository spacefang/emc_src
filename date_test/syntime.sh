#date -d now +'%Y %m %d %H %m %S'>curdate.txt|scp curdate.txt root@192.168.3.10:/mnt/EMMC/emc_test/|ssh -t -p 22 root@192.168.3.10 'cd /mnt/EMMC/emc_test/&&cat curdate.txt |xargs ./date_test'

#!/bin/sh

# 脚本功能：获取当前PC的时间，并将其分别设置到两台远程设备上

# --- 第1步：在本地创建一个包含当前时间的文件 ---
echo "正在获取当前时间并生成时间文件 curdate.txt..."
date -d now +'%Y %m %d %H %M %S' > curdate.txt

# --- 第2步：处理第一台设备 (192.168.3.10) ---
echo "正在为 192.168.3.10 同步时间..."
# 将时间文件复制到第一台设备
scp curdate.txt root@192.168.3.10:/mnt/EMMC/emc_test/ && \
# 登录到第一台设备，读取文件内容，并执行时间设置程序
ssh -t -p 22 root@192.168.3.10 'cd /mnt/EMMC/emc_test/ && cat curdate.txt | xargs ./date_test'
echo "192.168.3.10 时间同步完成。"

echo # 打印一个空行，方便区分

# --- 第3步：处理第二台设备 (192.168.3.20) ---
echo "正在为 192.168.3.20 同步时间..."
# 将时间文件复制到第二台设备
scp curdate.txt root@192.168.3.20:/mnt/EMMC/emc_test/ && \
# 登录到第二台设备，读取文件内容，并执行时间设置程序
ssh -t -p 22 root@192.168.3.20 'cd /mnt/EMMC/emc_test/ && cat curdate.txt | xargs ./date_test'
echo "192.168.3.20 时间同步完成。"

echo # 打印一个空行

# --- 第4步：清理本地的时间文件 ---
rm curdate.txt
echo "全过程结束。"