// chess_threat.h — библиотека проверки угрозы белому королю
// от чёрной ладьи и чёрного слона (задача 2.25, А. Г. Юркин).
// Автор: Егоров И., группа 4ИБ2, ITHub.
#pragma once
#include <string>

namespace chess {

// Клетка доски: x — вертикаль (a..h -> 1..8), y — горизонталь (1..8).
struct Square {
    int x = 0;
    int y = 0;
    bool operator==(const Square& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Square& o) const { return !(*this == o); }
};

enum class Threat { None, Rook, Bishop, Both };

// Математическая модель задачи: позиция K, R, B и признаки угрозы TR, TB.
class ThreatAnalyzer {
public:
    // Разбор шахматной нотации ("e4"). Возвращает false при ошибке.
    static bool parseSquare(const std::string& s, Square& sq);
    // Лежит ли клетка p строго между a и c на одной линии.
    static bool isBetween(const Square& a, const Square& c, const Square& p);

    ThreatAnalyzer(Square king, Square rook, Square bishop);

    bool isValid() const;        // фигуры на разных клетках
    bool rookThreat() const;     // TR = Lr ∧ ¬Между(R, K, B)
    bool bishopThreat() const;   // TB = Lb ∧ ¬Между(B, K, R)
    Threat threat() const;
    std::string describe() const;  // текст ответа

private:
    Square k_, r_, b_;
};

// Версия библиотеки (для проверки подмены .so без пересборки программы).
const char* libraryVersion();

}  // namespace chess
