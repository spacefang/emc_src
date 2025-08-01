
#第一部分:变量定义区
#1.临时修改PATH环境变量
PATH := $(PATH):/work/code/hl_src/vendor/tools/host/huanglong/toolchains/aarch64-v100-linux/bin

#2.定义交叉编译工具链的前缀
CROSS_COMPILE=aarch64-v100-linux-

#3.定义C编译器
CC=$(CROSS_COMPILE)gcc

#4.定义链接器
LD=$(CC)

#5.定义编译器标志
CFLAGS=-Wall
#什么是编译器标志?
#编译器标志是传递给编译器的选项，用于控制编译过程的行为。
#-Wall: 启用所有警告信息，会对代码进行审查，如果有问题，会在终端打印出相应的警告信息。
#-O2: 优化级别，-O2表示进行中等程度的

#6.定义链接器标志
LDFLAGS = -L/work/code/hl_src/vendor/tools/host/huanglong/toolchains/aarch64-v100-linux/target/lib -static
#什么是"链接器"和"链接器标志"?
#在C语言的编译过程中，有两步主要操作：
#1.编译(compile):编译器(gcc)将源代码(.c文件)转换为目标文件(.o文件)。
#2.链接(link):链接器(ld)将一个或多个目标文件(.o文件)和库文件(.a或.so文件)合并成一个可执行文件。
#链接器标志:在链接阶段，链接器需要知道如何处理目标文件和库文件。链接器标志提供了这些信息。
#-L: 指定库文件的搜索路径。 这里是把交叉编译工具链里的ARM目标库文件,加入到库文件搜索列表里
#-static: 强制链接器生成静态链接的可执行文件，而不是动态链接的可执行文件。
#静态链接和动态链接的区别:
#静态链接: 在编译时将所有需要的库文件合并到可执行文件中，生成的可执行文件较大，但不依赖于外部库文件。
#动态链接: 在运行时加载需要的库文件，生成的可执行文件较小，但依赖于外部库文件。

#7.定义源代码和目标文件目录
SRC_DIR := ./src
OBJ_DIR := ./obj

#8.自动查找所有源文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
#wildcard函数(make提供的一个内置函数):给一个通配符的路径模式，就会返回匹配该模式的所有文件名列表。
#$(SRC_DIR)/*.c:表示在SRC_DIR目录下查找所有.c文件

#9.根据源文件自动生成所有目标文件和名字
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
#$(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o):表示将SRCS中的每个.c文件转换为对应的.o文件，并放在OBJ_DIR目录下。

#10.定义最终的执行者
# Executable name
EXEC = led_test

#第二部分:规则定义区
# Default target
#1.默认入口规则(all)
all: $(EXEC)
# Rule to make the executable
#2.链接规则:如何生成可执行文件
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)
#展开就是:
#aarch64-v100-linux-gcc obj/main.o -o led_test -L/work/code/hl_src/vendor/tools/host/huanglong/toolchains/aarch64-v100-linux/target/lib -static

#3.编译规则:如何从.c文件生成.o文件
# Rule to make the object file
#%.o: %.c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	@$(CC) $(CFLAGS) -c $< -o $@
#	@表示在执行命令时不显示该命令本身
#	$(CC):使用定义的C编译器
# $<:表示第一个依赖文件，这里是.c文件(比如src/main.c)
# $@:表示目标文件，这里是.o文件(比如obj/main.o)
# -c:表示只编译，不链接，生成目标文件(.o文件)
#展开就是:@aarch64-v100-linux-gcc -Wall -c src/main.c -o obj/main.o
#规则的基本结构:
#目标(Target): 依赖(Dependencies)
#	命令(Command)


# Clean up
#.PHONY:clean 这句是声明clean是一个伪目标，不是一个真实的文件名
.PHONY: clean
#清理规则:删除所有生成的目标文件和可执行文件
clean:
	rm -f $(OBJS) $(EXEC)

#方便后期调式，通过make info命令查看变量的值
info:
	@echo "$(SRCS) $(OBJS) $(EXEC)"
	@echo "$(PATH)"

