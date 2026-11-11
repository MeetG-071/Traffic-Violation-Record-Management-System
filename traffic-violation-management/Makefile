# Traffic Violation Record Management System - build automation
# Targets: all, run, test, clean

CC       := gcc
CXX      := g++
WARNINGS := -Wall -Wextra -Wpedantic
CFLAGS   := -std=c11 $(WARNINGS) -O2 -Iinclude -MMD -MP
CXXFLAGS := -std=c++17 $(WARNINGS) -O2 -Iinclude -Itests -MMD -MP
LDFLAGS  :=
GTEST_LIBS := -lgtest -lgtest_main -pthread

SRC_DIR   := src
TEST_DIR  := tests
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj

APP_BIN   := $(BUILD_DIR)/tvrms
TEST_BIN  := $(BUILD_DIR)/run_tests

APP_SRCS  := $(wildcard $(SRC_DIR)/*.c)
APP_OBJS  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(APP_SRCS))
LIB_OBJS  := $(filter-out $(OBJ_DIR)/main.o,$(APP_OBJS))
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/tests/%.o,$(TEST_SRCS))

DATA_FILE   := data/violations.txt
SAMPLE_FILE := data/sample_data.txt

.PHONY: all run test clean

all: $(APP_BIN)

$(APP_BIN): $(APP_OBJS)
	$(CC) $(APP_OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_BIN): $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $(LIB_OBJS) $(TEST_OBJS) -o $@ $(GTEST_LIBS)

# Copies the sample data on first run so the sample file itself is never modified.
run: all
	@test -f $(DATA_FILE) || cp $(SAMPLE_FILE) $(DATA_FILE)
	./$(APP_BIN) $(DATA_FILE)

test: $(TEST_BIN)
	./$(TEST_BIN)

# Removes build output and the generated report. data/violations.txt (your working
# database) is kept on purpose; delete it manually to reset to the sample data.
clean:
	rm -rf $(BUILD_DIR) $(TEST_BIN) data/summary_report.txt

-include $(APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d)
