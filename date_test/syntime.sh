date -d now +'%Y %m %d %H %m %S'>curdate.txt|scp curdate.txt root@192.168.3.10:/mnt/EMMC/emc_test/|ssh -t -p 22 root@192.168.3.10 'cd /mnt/EMMC/emc_test/&&cat curdate.txt |xargs ./date_test'

