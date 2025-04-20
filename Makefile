SHELL := /bin/bash

# Directories
SRC       := src
BUILD     := build
BIN       := bin
INC       := include

# Compiler and flags
CC        := gcc
CFLAGS := -Wall -Wextra \
          -Wpedantic \
          -I$(INC) -g
LDFLAGS   := -lncurses

# Source files and objects
SRCS      := $(wildcard $(SRC)/*.c)
OBJS      := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SRCS))
TARGET    := $(BIN)/main

.PHONY: all clean run generate_code

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD) $(BIN):
	mkdir -p $@

# cleaning the compiled filess
clean:
	rm -rf $(BUILD) $(BIN) code.txt

# run the program
run: all
	$(TARGET)

# Generate a code snapshot
generate_code:
	@echo "===== PROJECT TREE =====" > code.txt
	@tree -I '.*' >> code.txt
	@echo -e "\n===== FILE CONTENTS =====" >> code.txt
	@find . -type f \
		! -path "./.*" \
		! -path "*/.*" \
		! -name code.txt | while read -r file; do \
		echo "----- $$file -----" >> code.txt; \
		cat "$$file" >> code.txt; \
		echo "" >> code.txt; \
	done
