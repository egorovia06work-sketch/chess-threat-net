// chess_threat.cpp — реализация библиотеки (см. chess_threat.h).
#include "chess_threat.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace chess {

const char* libraryVersion() { return "2.0"; }

// Перевод клетки в нотацию: (5, 1) -> "e1".
static std::string toNotation(const Square& s) {
    return std::string(1, static_cast<char>('a' + s.x - 1)) +
           static_cast<char>('0' + s.y);
}

bool ThreatAnalyzer::parseSquare(const std::string& s, Square& sq) {
    if (s.size() != 2) return false;
    char f = static_cast<char>(std::tolower(static_cast<unsigned char>(s[0])));
    char r = s[1];
    if (f < 'a' || f > 'h' || r < '1' || r > '8') return false;
    sq.x = f - 'a' + 1;
    sq.y = r - '0';
    return true;
}

bool ThreatAnalyzer::isBetween(const Square& a, const Square& c, const Square& p) {
    int dx = c.x - a.x, dy = c.y - a.y;
    int sx = (dx > 0) - (dx < 0), sy = (dy > 0) - (dy < 0);
    int n = std::max(std::abs(dx), std::abs(dy));
    for (int i = 1; i < n; ++i)
        if (a.x + i * sx == p.x && a.y + i * sy == p.y) return true;
    return false;
}

ThreatAnalyzer::ThreatAnalyzer(Square king, Square rook, Square bishop)
    : k_(king), r_(rook), b_(bishop) {}

bool ThreatAnalyzer::isValid() const {
    return k_ != r_ && k_ != b_ && r_ != b_;
}

bool ThreatAnalyzer::rookThreat() const {
    bool line = (k_.x == r_.x) || (k_.y == r_.y);
    return line && !isBetween(r_, k_, b_);
}

bool ThreatAnalyzer::bishopThreat() const {
    bool diag = std::abs(k_.x - b_.x) == std::abs(k_.y - b_.y);
    return diag && !isBetween(b_, k_, r_);
}

Threat ThreatAnalyzer::threat() const {
    bool tr = rookThreat(), tb = bishopThreat();
    if (tr && tb) return Threat::Both;
    if (tr) return Threat::Rook;
    if (tb) return Threat::Bishop;
    return Threat::None;
}

std::string ThreatAnalyzer::describe() const {
    switch (threat()) {
        case Threat::Both:
            return "Королю угрожают ладья с " + toNotation(r_) + " и слон с " +
                   toNotation(b_) + " (двойной шах).";
        case Threat::Rook:   return "Королю угрожает ладья с " + toNotation(r_) + ".";
        case Threat::Bishop: return "Королю угрожает слон с " + toNotation(b_) + ".";
        default:             return "Угрозы королю нет.";
    }
}

}  // namespace chess
