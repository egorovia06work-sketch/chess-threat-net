# chess-threat-net — клиент-серверная версия задачи 2.25

Задача 2.25 (А. Г. Юркин): белый король, чёрная ладья и слон — есть ли угроза королю и от кого.
Бизнес-логика вынесена в сетевой сервис (backend), клиент передаёт ему позиции по TCP.

```
lib/      бизнес-логика: модель задачи (chess_threat) и протокол CTP/1.0 (protocol) — libchessnet.a
server/   TCP-сервер chess_server (поток на каждого клиента), свой Makefile
client/   сетевой клиент chess_client (Linux; Windows — make windows), свой Makefile
tests/    модульные тесты протокола и интеграционный тест клиент–сервер
docs/     описание протокола
```

## Сборка и запуск (Linux / WSL)

```bash
make                 # lib + server + client
make test            # модульные и интеграционные тесты

./server/chess_server -p 5025                 # терминал 1
./client/chess_client -H 127.0.0.1 e1 e8 h4   # терминал 2
./client/chess_client -f tests/positions.txt  # пакет позиций
./client/chess_client                         # интерактивно
printf 'CHECK e1 e8 a1\nQUIT\n' | nc 127.0.0.1 5025   # без клиента
```

Каждый проект собирается и отдельно: `make -C server`, `make -C client`.
Клиент для Windows: `make -C client windows` (нужен MinGW-w64) → `chess_client.exe`.

Протокол: [docs/protocol.md](docs/protocol.md).

Автор: Егоров И., группа 4ИБ2, ITHub. Преподаватель: Сухов А. А.

## Git hooks

Исходники хуков — в каталоге `githooks/`, установка в `.git/hooks`: `make hooks`.

* **pre-commit** — выводит три последних изменения; проверяет пробелы и маркеры
  конфликтов, сборку, модульные тесты и ошибки cppcheck в изменённых файлах C++.
  При ошибке коммит отменяется.
* **post-commit** — выводит три последних изменения (`git log -3`) и запускает
  статический анализатор cppcheck для файлов C++, изменённых в коммите;
  отчёт — `.git/cppcheck-report.txt`.

Полный анализ проекта: `make cppcheck`.
