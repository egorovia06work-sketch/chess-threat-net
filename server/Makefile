# Сервер: make, make run [PORT=5025], make clean
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
LIBDIR   := ../lib
LIB      := $(LIBDIR)/build/libchessnet.a
BUILD    := build
TARGET   := chess_server
PORT     ?= 5025

.PHONY: all run clean FORCE
all: $(TARGET)

# библиотека собирается своим Makefile (make -C ../lib)
$(LIB): FORCE
	$(MAKE) -C $(LIBDIR)
FORCE:

$(BUILD)/server.o: src/server.cpp $(LIBDIR)/protocol.h
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -I$(LIBDIR) -c $< -o $@

$(TARGET): $(BUILD)/server.o $(LIB)
	$(CXX) $^ -pthread -o $@

run: $(TARGET)
	./$(TARGET) -p $(PORT)

clean:
	rm -rf $(BUILD) $(TARGET)
