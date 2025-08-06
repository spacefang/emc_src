# SUBDIRS := date_test led_test rs485_test_rcv rs485_test_snd udp_client udp_server usb_test
 
# .PHONY: all
 
# all:
# 	for dir in $(SUBDIRS); do \
# 		$(MAKE) -C $$dir -j1 all || exit $$?; \
# 	done
# clean:
# 	for dir in $(SUBDIRS); do \
# 		$(MAKE) -C $$dir -j1 clean || exit $$?; \
# 	done

# ====================================================================
#          Unified Makefile for EMC Test Project
# ====================================================================

# 1. 配置您的交叉编译工具链路径
# 将此路径修改为您环境中的实际路径
TOOLCHAIN_PATH := /mnt/d/work/code/hl_src/vendor/tools/host/huanglong/toolchains/aarch64-v100-linux
# ====================================================================

# 自动配置环境变量和编译器
export PATH := $(TOOLCHAIN_PATH)/bin:$(PATH)
CROSS_COMPILE=aarch64-v100-linux-
CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -I./Shared_Memory # 添加-I指令，让所有程序都能直接找到shared_data.h
LDFLAGS = -L$(TOOLCHAIN_PATH)/target/lib -static

# 定义所有需要编译的程序
# 我们把所有最终生成的可执行文件都放在一个新的 bin 目录下
TARGETS := \
	bin/rs485_test_snd \
	bin/rs485_test_rcv \
	bin/udp_client \
	bin/udp_server \
	bin/reporter \
	bin/reporter_rcv
# 	bin/led_test \
# 	bin/date_test \
# 	bin/usb_test \
# 	bin/watchdog_test

# 默认目标，编译所有程序
all: $(TARGETS)

# 创建目标目录
$(shell mkdir -p bin)
$(shell mkdir -p obj)

# --- 单个程序的编译规则 ---

# 编译 reporter
bin/reporter: obj/reporter.o Shared_Memory/shared_data.h
	$(CC) obj/reporter.o -o $@ $(LDFLAGS)
obj/reporter.o: Shared_Memory/reporter.c Shared_Memory/shared_data.h
	$(CC) $(CFLAGS) -c $< -o $@

# ================== 新增 reporter_rcv 的编译规则 ==================
bin/reporter_rcv: obj/reporter_rcv.o
	$(CC) obj/reporter_rcv.o -o $@ $(LDFLAGS)
obj/reporter_rcv.o: Shared_Memory/reporter_rcv.c
	$(CC) $(CFLAGS) -c $< -o $@
# ================================================================

# 编译 rs485_test_snd
bin/rs485_test_snd: obj/rs485_snd_main.o obj/rs485_snd_crc.o Shared_Memory/shared_data.h
	$(CC) $^ -o $@ $(LDFLAGS)
obj/rs485_snd_main.o: rs485_test_snd/src/main.c Shared_Memory/shared_data.h
	$(CC) $(CFLAGS) -c $< -o $@
obj/rs485_snd_crc.o: rs485_test_snd/src/crc.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译 rs485_test_rcv
bin/rs485_test_rcv: obj/rs485_rcv_main.o obj/rs485_rcv_crc.o
	$(CC) $^ -o $@ $(LDFLAGS)
obj/rs485_rcv_main.o: rs485_test_rcv/src/main.c
	$(CC) $(CFLAGS) -c $< -o $@
obj/rs485_rcv_crc.o: rs485_test_rcv/src/crc.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译 udp_client
bin/udp_client: obj/udp_client_main.o obj/udp_client_crc.o Shared_Memory/shared_data.h
	$(CC) $^ -o $@ $(LDFLAGS)
obj/udp_client_main.o: udp_client/src/main.c Shared_Memory/shared_data.h
	$(CC) $(CFLAGS) -c $< -o $@
obj/udp_client_crc.o: udp_client/src/crc.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译 udp_server
bin/udp_server: obj/udp_server_main.o obj/udp_server_crc.o
	$(CC) $^ -o $@ $(LDFLAGS)
obj/udp_server_main.o: udp_server/src/main.c
	$(CC) $(CFLAGS) -c $< -o $@
obj/udp_server_crc.o: udp_server/src/crc.c
	$(CC) $(CFLAGS) -c $< -o $@
    
# ... (其他程序的编译规则，与上面类似) ...
# 为了简洁，此处省略了led_test, date_test, usb_test, watchdog_test的规则
# 您可以按照上面的格式轻松添加它们

# --- 清理规则 ---
.PHONY: clean all

clean:
	rm -rf obj/* bin/*