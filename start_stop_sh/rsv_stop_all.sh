#!/bin/sh
echo "Stopping all loopback services on receiver..."
killall rs485_test_rcv
killall udp_server
echo "Receiver services stopped."