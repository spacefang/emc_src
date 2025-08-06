#!/bin/sh
echo "Starting all test processes on sender..."
# 启动所有串口发送进程
./rs485_test_snd 0 &
./rs485_test_snd 1 &
./rs485_test_snd 2 &
./rs485_test_snd 3 &
./rs485_test_snd 4 &
./rs485_test_snd 5 &
./rs485_test_snd 6 &
./rs485_test_snd 7 &
# ... (添加其他所有 rs485_test_snd 的启动命令)

# 启动所有网口客户端进程
./udp_client 192.168.1.10 9091 192.168.1.20 9092 &
./udp_client 192.168.2.10 9091 192.168.2.20 9092 &
./udp_client 192.168.3.10 9091 192.168.3.20 9092 &
./udp_client 192.168.4.10 9091 192.168.4.20 9092 &
./udp_client 192.168.5.10 9091 192.168.5.20 9092 &
./udp_client 192.168.6.10 9091 192.168.6.20 9092 &
./udp_client 192.168.7.10 9091 192.168.7.20 9092 &
./udp_client 192.168.7.10 9091 192.168.7.20 9092 &
# ... (添加其他所有 udp_client 的启动命令)
echo "Sender processes started."