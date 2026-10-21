# Сетевое взаимодействие: сервер и клиент задачи 2.25.
#   make            — собрать библиотеку, сервер и клиент
#   make test       — модульные (без сети) и интеграционные тесты
#   make clean
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2

.PHONY: all lib server client test unit integration clean
all: server client

lib:
	$(MAKE) -C lib
server: lib
	$(MAKE) -C server
client:
	$(MAKE) -C client

tests/test_protocol: tests/test_protocol.cpp lib/build/libchessnet.a
	$(CXX) $(CXXFLAGS) -Ilib $^ -o $@

lib/build/libchessnet.a: lib

unit: lib tests/test_protocol
	./tests/test_protocol

integration: all
	./tests/integration.sh

test: unit integration

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C server clean
	$(MAKE) -C client clean
	rm -f tests/test_protocol tests/server.log
