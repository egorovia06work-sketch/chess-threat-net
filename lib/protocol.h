// protocol.h — прикладной протокол «шахматного» сервиса (задача 2.25).
//
// Текстовый построчный протокол поверх TCP, кодировка UTF-8, строки
// оканчиваются '\n'. Клиент отправляет команду, сервер отвечает одной строкой.
//
//   CHECK <король> <ладья> <слон>  -> OK <NONE|ROOK|BISHOP|BOTH> <текст ответа>
//   PING                           -> PONG
//   STATS                          -> OK requests=<n> clients=<n> errors=<n>
//   VERSION                        -> OK chess-threat-server <версия протокола> lib <версия>
//   HELP                           -> OK <список команд>
//   QUIT                           -> BYE (сервер закрывает соединение)
//   ошибка                         -> ERR <код> <описание>
//
// Коды ошибок: 400 — неизвестная команда или неверное число аргументов,
//              422 — некорректная позиция (клетка вне доски, фигуры на одной клетке).
#pragma once
#include <atomic>
#include <string>

namespace chess::proto {

constexpr const char* kProtocolVersion = "CTP/1.0";   // Chess Threat Protocol
constexpr int kDefaultPort = 5025;                    // задача 2.25 :)

// Счётчики сервера (общие для всех соединений).
struct Stats {
    std::atomic<long> requests{0};
    std::atomic<long> clients{0};
    std::atomic<long> errors{0};
};

// Результат обработки одной строки запроса.
struct Reply {
    std::string text;     // строка ответа без '\n'
    bool close = false;   // закрыть соединение после ответа
};

// Обработка одной строки запроса — вся бизнес-логика сервера.
// Не зависит от сети, поэтому тестируется отдельно.
Reply handle(const std::string& line, Stats& stats);

}  // namespace chess::proto
