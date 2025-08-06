#!/bin/sh
echo "Stopping all test processes on sender..."
killall rs485_test_snd
killall udp_client
echo "Sender processes stopped."