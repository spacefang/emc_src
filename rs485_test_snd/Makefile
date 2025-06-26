
PATH := $(PATH):/work/code/hl_src/vendor/tools/host/huanglong/toolchains/aarch64-v100-linux/bin

# 定义交叉编译工具链的前缀
CROSS_COMPILE=aarch64-v100-linux-

# 定义C编译器
CC=$(CROSS_COMPILE)gcc

# 定义链接器
LD=$(CC)

# 定义编译器标志
CFLAGS=-Wall

LDFLAGS = -L/work/code/hl_src/vendor/tools/host/huanglong/toolchains/aarch64-v100-linux/target/lib -static



SRC_DIR := ./src
OBJ_DIR := ./obj

SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# Executable name
EXEC = rs485_test_snd


# Default target
all: $(EXEC)

# Rule to make the executable
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

# Rule to make the object file
#%.o: %.c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	@$(CC) $(CFLAGS) -c $< -o $@

# Clean up
.PHONY: clean
clean:
	rm -f $(OBJS) $(EXEC)
info:
	@echo "$(SRCS) $(OBJS) $(EXEC)"
	@echo "$(PATH)"

