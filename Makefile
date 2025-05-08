SHELL := /bin/bash

# === Répertoires ===
SRC   := src
BUILD := build
BIN   := bin
INC   := include

# === Compilation ===
CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -I$(INC) -g
LDFLAGS := -lncurses

# === Fichiers ===
SRCS   := $(wildcard $(SRC)/*.c)
OBJS   := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SRCS))
TARGET := $(BIN)/main

# === Couleurs ===
RESET  := \033[0m
BOLD   := \033[1m
RED    := \033[31m
GREEN  := \033[32m
YELLOW := \033[33m
BLUE   := \033[34m
CYAN   := \033[36m

.PHONY: all clean run generate_code banner

# === Effet progressif ===
define progress
	@sleep 0.2 && echo -n "." && sleep 0.2 && echo -n "." && sleep 0.2 && echo "."
endef

# === Build principal ===
all: banner $(TARGET)
	@$(call progress)
	@echo -e "$(GREEN)✅ Build terminé avec succès.$(RESET)\n"

$(TARGET): $(OBJS) | $(BIN)
	@echo -e "$(CYAN)\n🔗 ===== Linking Objects =====$(RESET)"
	@$(call progress)
	@$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	@echo -e "$(YELLOW)------------------------------------------------------$(RESET)"

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	@echo -e "$(BLUE)🔨 Compilation de $<...$(RESET)"
	@$(call progress)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD) $(BIN):
	@mkdir -p $@

# === Nettoyage ===
clean:
	@echo -e "$(RED)\n🧹 Nettoyage du projet...$(RESET)"
	@$(call progress)
	@rm -rf $(BUILD) $(BIN) code.txt
	@$(call progress)
	@echo -e "$(YELLOW)🧼 Fichiers supprimés avec succès.$(RESET)"
	@echo -e "$(YELLOW)------------------------------------------------------$(RESET)"

# === Exécution ===
run: all
	@echo -e "$(GREEN)\n🚀 Lancement de l'application :$(RESET)"
	@$(call progress)
	@$(TARGET)
	@echo -e "$(YELLOW)------------------------------------------------------$(RESET)"

# === Génération snapshot ===
generate_code:
	@echo -e "$(CYAN)\n📄 Génération de 'code.txt'...$(RESET)"
	@$(call progress)
	@echo "===== PROJECT TREE =====" > code.txt
	@tree -I '.*' >> code.txt
	@$(call progress)
	@echo -e "\n===== FILE CONTENTS =====" >> code.txt
	@find . -type f \
		! -path "./.*" \
		! -path "*/.*" \
		! -name code.txt | while read -r file; do \
		echo "----- $$file -----" >> code.txt; \
		cat "$$file" >> code.txt; \
		echo "" >> code.txt; \
	done
	@$(call progress)
	@echo -e "$(GREEN)📚 'code.txt' généré avec succès.$(RESET)"
	@echo -e "$(YELLOW)------------------------------------------------------$(RESET)"
# === Bannière ===
banner:
	@echo -e "$(BOLD)$(BLUE)"
	@echo "  ____    _    ____   ____    _    ____ ____   ___  _   _ _   _ _____"
	@echo " / ___|  / \  |  _ \ / ___|  / \  / ___/ ___| / _ \| \ | | \ | | ____|"
	@echo "| |     / _ \ | |_) | |     / _ \ \___ \___ \| | | |  \| |  \| |  _|"
	@echo "| |___ / ___ \|  _ <| |___ / ___ \ ___) |__) | |_| | |\  | |\  | |___"
	@echo " \____/_/   \_\_| \_\\____/_/   \_\____/____/ \___/|_| \_|_| \_|_____|"
	@$(call progress)
	@echo -e "\n        Bienvenue dans Carcassonne !$(RESET)"
	@echo -e "$(YELLOW)======================================================$(RESET)\n"
