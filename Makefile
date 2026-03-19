# ------------------- 配置 -------------------
BPF_DIR := bpf
SRC_DIR := src
BUILD_DIR := build
BPF_OBJ := $(BUILD_DIR)/sysmon_bpf.o
SKEL_H  := $(BUILD_DIR)/sysmon_bpf.skel.h
USER_OBJ := $(BUILD_DIR)/sysmon_user.o $(BUILD_DIR)/log.o \
	        $(BUILD_DIR)/event_handler.o $(BUILD_DIR)/memory_leak.o \
	        $(BUILD_DIR)/signals.o $(BUILD_DIR)/comm_filter.o
USER_BIN := $(BUILD_DIR)/sysmon_core
CLANG ?= clang
CFLAGS_USER := -g -O2 -Wall -I$(BUILD_DIR) -I$(SRC_DIR) -I$(BPF_DIR) -lelf -lbpf
# ------------------- 默认目标 -------------------
all: $(USER_BIN)
# ------------------- 生成 vmlinux.h -------------------
$(BUILD_DIR)/vmlinux.h:
	@echo "Dumping /sys/kernel/btf/vmlinux to vmlinux.h (may require sudo)"
	./scripts/gen_vmlinux_h.sh $@
# ------------------- BPF 编译 -------------------
$(BPF_OBJ): $(BPF_DIR)/sysmon_bpf.c $(BPF_DIR)/helpers.h $(BPF_DIR)/types.h $(BUILD_DIR)/vmlinux.h | $(BUILD_DIR)
	$(CLANG) -O2 -g -Wall -target bpf -I. -I$(BPF_DIR) -c $< -o $@
# ------------------- 生成 skeleton header -------------------
$(SKEL_H): $(BPF_OBJ)
	bpftool gen skeleton $< > $@
# ------------------- 用户态编译 -------------------
$(BUILD_DIR)/sysmon_user.o: $(SRC_DIR)/sysmon_user.c $(SRC_DIR)/types.h $(SRC_DIR)/log.h $(SRC_DIR)/event_handler.h $(SRC_DIR)/signals.h $(SRC_DIR)/memory_leak.h $(SKEL_H) | $(BUILD_DIR)
	$(CLANG) $(CFLAGS_USER) -c $< -o $@
$(BUILD_DIR)/log.o: $(SRC_DIR)/log.c $(SRC_DIR)/log.h | $(BUILD_DIR)
	$(CLANG) $(CFLAGS_USER) -c $< -o $@
$(BUILD_DIR)/event_handler.o: $(SRC_DIR)/event_handler.c $(SRC_DIR)/event_handler.h $(SRC_DIR)/types.h $(SRC_DIR)/memory_leak.h | $(BUILD_DIR)
	$(CLANG) $(CFLAGS_USER) -c $< -o $@
$(BUILD_DIR)/memory_leak.o: $(SRC_DIR)/memory_leak.c $(SRC_DIR)/memory_leak.h $(SRC_DIR)/log.h | $(BUILD_DIR)
	$(CLANG) $(CFLAGS_USER) -c $< -o $@
$(BUILD_DIR)/signals.o: $(SRC_DIR)/signals.c $(SRC_DIR)/signals.h | $(BUILD_DIR)
	$(CLANG) $(CFLAGS_USER) -c $< -o $@
$(BUILD_DIR)/comm_filter.o: $(SRC_DIR)/comm_filter.c $(SRC_DIR)/comm_filter.h | $(BUILD_DIR)
	$(CLANG) $(CFLAGS_USER) -c $< -o $@
# ------------------- 链接生成可执行文件 -------------------
$(USER_BIN): $(USER_OBJ)
	$(CLANG) $(CFLAGS_USER) $^ -o $@
# ------------------- 创建 build 目录 -------------------
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
# ------------------- 清理 -------------------
clean:
	rm -rf $(BUILD_DIR) *.o $(USER_BIN)
.PHONY: all clean
