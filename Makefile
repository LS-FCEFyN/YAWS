CXX      := g++
CXXFLAGS := -std=c++20 -Os -flto -fno-rtti -fno-ident -fno-use-cxa-atexit -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wpedantic -Iheaders
LDFLAGS  := -pthread -flto -Wl,--gc-sections,--as-needed,--strip-all,-O2

SRC_DIR  := src
OBJ_DIR  := build
TARGET   := yaws

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)