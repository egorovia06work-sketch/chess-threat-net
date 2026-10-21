# Библиотека бизнес-логики: модель задачи 2.25 + прикладной протокол.
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
AR       ?= ar
BUILD    := build
SRC      := chess_threat.cpp protocol.cpp
OBJ      := $(SRC:%.cpp=$(BUILD)/%.o)
LIB      := $(BUILD)/libchessnet.a

.PHONY: all clean
all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

$(BUILD)/%.o: %.cpp chess_threat.h protocol.h
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
