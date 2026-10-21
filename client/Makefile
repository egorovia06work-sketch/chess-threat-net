# Клиент: make, make run [HOST=127.0.0.1 PORT=5025 ARGS="e1 e8 h4"], make clean
# Клиенту не нужна бизнес-логика: он только передаёт данные серверу.
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
BUILD    := build
TARGET   := chess_client
HOST     ?= 127.0.0.1
PORT     ?= 5025
ARGS     ?= e1 e8 h4

.PHONY: all run clean windows
all: $(TARGET)

$(BUILD)/client.o: src/client.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(BUILD)/client.o
	$(CXX) $^ -o $@

run: $(TARGET)
	./$(TARGET) -H $(HOST) -p $(PORT) $(ARGS)

# Клиент для Windows (кросс-компиляция MinGW-w64 из Linux/WSL)
MINGW    ?= x86_64-w64-mingw32-g++
windows: $(TARGET).exe
$(TARGET).exe: src/client.cpp
	$(MINGW) $(CXXFLAGS) -static $< -lws2_32 -o $@

clean:
	rm -rf $(BUILD) $(TARGET) $(TARGET).exe
