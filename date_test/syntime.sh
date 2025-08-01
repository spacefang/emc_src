#date -d now +'%Y %m %d %H %M %S'>curdate.txt|scp curdate.txt root@192.168.3.10:/mnt/EMMC/emc_test/|ssh -t -p 22 root@192.168.3.10 'cd /mnt/EMMC/emc_test/&&cat curdate.txt |xargs ./date_test'
#1.生成时间
#date -d now +'%Y %m %d %H %M %S'>curdate.txt  重定向写入curdate.txt

#2.将时间文件复制到一体机
#scp curdate.txt root@192.168.3.10:/mnt/EMMC/emc_test/

#3.登录一体机，执行时间设置脚本
#ssh -t -p 22 root@192.168.3.10
#'cd /mnt/EMMC/emc_test/&&cat curdate.txt |xargs ./date_test'   '...'引号里的内容，是在一体机上需要执行的命令
#cd /mnt/EMMC/emc_test/ 进入目录
#cat curdate.txt |xargs ./date_test 读取curdate.txt文件内容，并将内容作为参数传递给date_test程序
#xargs 是一个命令行工具，用于将标准输入转换为命令行参数。它从标准输入读取数据，并将这些数据作为参数传递给指定的命令。
#xargs 的常见用途是处理管道输出，将其转换为命令行参数。例如，`cat file.txt | xargs command` 会将 `file.txt` 中的内容作为参数传递给 `command` 命令。
#xargs 的好处是可以处理任意数量的参数，而不受命令行长度的限制。它还可以自动处理空格和特殊字符，使得命令行参数的传递更加灵活。
#本例中，就是将接收到的时间字符串（"2025 07 02 19 01 54"）拆分开，作为参数传递给 ./date_test 程序。
#最终执行的效果就等同于：./date_test 2025 07 02 19 01 54。

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
