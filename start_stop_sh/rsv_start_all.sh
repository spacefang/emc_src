#!/bin/sh
echo "Starting all loopback services on receiver..."
./rs485_test_rcv 0 &
./rs485_test_rcv 1 &
./rs485_test_rcv 2 &
./rs485_test_rcv 3 &
./rs485_test_rcv 4 &
./rs485_test_rcv 5 &
./rs485_test_rcv 6 &
./rs485_test_rcv 7 &


./udp_server 192.168.1.20 9092 &
./udp_server 192.168.2.20 9092 &
./udp_server 192.168.3.20 9092 &
./udp_server 192.168.4.20 9092 &
./udp_server 192.168.5.20 9092 &
./udp_server 192.168.6.20 9092 &
./udp_server 192.168.7.20 9092 &
./udp_server 192.168.8.20 9092 &
echo "Receiver services started."