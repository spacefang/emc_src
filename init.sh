#scp rs485_test_rcv/rs485_test_rcv root@192.168.3.10:/mnt/EMMC/emc_test/rs485_test_rcv
echo -e "\nstart scp files to 192.168.3.10\n"
scp rs485_test_snd/rs485_test_snd root@192.168.3.10:/mnt/EMMC/emc_test/rs485_test_snd
#scp udp_server/udp_server root@192.168.3.10:/mnt/EMMC/emc_test/udp_server
scp udp_client/udp_client root@192.168.3.10:/mnt/EMMC/emc_test/udp_client
scp usb_test/usb_test root@192.168.3.10:/mnt/EMMC/emc_test/usb_test
scp led_test/led_test root@192.168.3.10:/mnt/EMMC/emc_test/led_test
scp date_test/date_test root@192.168.3.10:/mnt/EMMC/emc_test/date_test
#
#

echo -e "\nstart scp files to 192.168.3.20\n"
scp rs485_test_rcv/rs485_test_rcv root@192.168.3.20:/mnt/EMMC/emc_test/rs485_test_rcv
#scp rs485_test_snd/rs485_test_snd root@192.168.3.20:/mnt/EMMC/emc_test/rs485_test_snd
scp udp_server/udp_server root@192.168.3.20:/mnt/EMMC/emc_test/udp_server
#scp udp_client/udp_client root@192.168.3.20:/mnt/EMMC/emc_test/udp_client
#scp usb_test/usb_test root@192.168.3.20:/mnt/EMMC/emc_test/usb_test
#scp led_test/led_test root@192.168.3.20:/mnt/EMMC/emc_test/led_test
scp date_test/date_test root@192.168.3.20:/mnt/EMMC/emc_test/date_test