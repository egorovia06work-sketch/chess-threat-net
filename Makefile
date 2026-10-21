# Сетевое взаимодействие: сервер и клиент задачи 2.25.
#   make            — собрать библиотеку, сервер и клиент
#   make test       — модульные (без сети) и интеграционные тесты
#   make hooks      — установить git hooks (pre-commit, post-commit) в .git/hooks
#   make cppcheck   — статический анализ всего проекта
#   make clean
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2

.PHONY: all lib server client test unit integration clean hooks cppcheck
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

# Git hooks: исходники хранятся в репозитории (githooks/), устанавливаются в .git/hooks
hooks:
	install -m 755 githooks/pre-commit githooks/post-commit .git/hooks/
	@stat -c '%A  %n' .git/hooks/pre-commit .git/hooks/post-commit

cppcheck:
	cppcheck --enable=warning,style,performance,portability --std=c++17 --language=c++ \
	         --inline-suppr --quiet -I lib lib server/src client/src tests

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C server clean
	$(MAKE) -C client clean
	rm -f tests/test_protocol tests/server.log
