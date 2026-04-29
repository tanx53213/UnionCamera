CC := arm-linux-gcc

TARGET := main_V1
SRC_DIR := Sourse
INC_DIR := Include

# 库路径（注意空格和特殊字符）
LIB_DIR := Library
LIBFONT := $(LIB_DIR)/libfont.a

SRCS := \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/TouchScreen4.c \
	$(SRC_DIR)/op_Boot_anim.c \
	$(SRC_DIR)/VideoPlayer3.c \
	$(SRC_DIR)/API_Camera2.c \
	$(SRC_DIR)/Draw_line.c \
	$(SRC_DIR)/test.c \
	$(SRC_DIR)/testtime.c \
	$(SRC_DIR)/get_weather.c

CFLAGS += -I$(INC_DIR) -Wall -Wextra
LDFLAGS += -lpthread -lm $(LIBFONT)   # 直接使用 .a 文件，不用 -l

.PHONY: all clean print

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

print:
	@echo "CC=$(CC)"
	@echo "SRCS=$(SRCS)"




# CC := arm-linux-gcc

# TARGET := main
# SRC_DIR := Sourse
# INC_DIR := Include


# SRCS := \
# 	$(SRC_DIR)/main.c \
# 	$(SRC_DIR)/TouchScreen4.c \
# 	$(SRC_DIR)/op_Boot_anim.c \
# 	$(SRC_DIR)/VideoPlayer3.c \
# 	$(SRC_DIR)/API_Camera2.c \
# 	$(SRC_DIR)/Draw_line.c \
# 	$(SRC_DIR)/test.c \
# 	$(SRC_DIR)/testtime.c \
# 	$(SRC_DIR)/get_weathe2.c

# CFLAGS += -I$(INC_DIR) -Wall -Wextra
# LDFLAGS += -lpthread -lm -lfont

# .PHONY: all clean print

# all: $(TARGET)

# $(TARGET): $(SRCS)
# 	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# clean:
# 	rm -f $(TARGET)

# print:
# 	@echo "CC=$(CC)"
# 	@echo "SRCS=$(SRCS)"
