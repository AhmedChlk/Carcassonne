TARGET = bin/main.exe
CC = gcc
CFLAGS = -Wall -g -Wextra

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

all : $(TARGET)

$(TARGET) : $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR) :
	mkdir -p $(BUILD_DIR)

$(BIN_DIR) :
	mkdir -p $(BIN_DIR)

clean :
	rm -f $(BUILD_DIR)/*.o $(TARGET)

run : 
	@./bin/main.exe