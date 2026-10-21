// protocol.cpp — реализация прикладного протокола (см. protocol.h).
#include "protocol.h"
#include "chess_threat.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace chess::proto {

static const char* threatName(Threat t) {
    switch (t) {
        case Threat::Rook:   return "ROOK";
        case Threat::Bishop: return "BISHOP";
        case Threat::Both:   return "BOTH";
        default:             return "NONE";
    }
}

static Reply error(Stats& s, int code, const std::string& msg) {
    ++s.errors;
    return {"ERR " + std::to_string(code) + " " + msg};
}

Reply handle(const std::string& line, Stats& stats) {
    ++stats.requests;
    std::istringstream in(line);
    std::vector<std::string> w;
    for (std::string t; in >> t;) w.push_back(t);
    if (w.empty()) return error(stats, 400, "пустой запрос");

    std::string cmd = w[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

    if (cmd == "PING") return {"PONG"};
    if (cmd == "QUIT") return {"BYE", true};
    if (cmd == "HELP")
        return {"OK команды: CHECK <король> <ладья> <слон>, PING, STATS, VERSION, HELP, QUIT"};
    if (cmd == "VERSION")
        return {std::string("OK chess-threat-server ") + kProtocolVersion + " lib " + libraryVersion()};
    if (cmd == "STATS")
        return {"OK requests=" + std::to_string(stats.requests.load()) +
                " clients=" + std::to_string(stats.clients.load()) +
                " errors=" + std::to_string(stats.errors.load())};
    if (cmd == "CHECK") {
        if (w.size() != 4) return error(stats, 400, "ожидается: CHECK <король> <ладья> <слон>");
        Square k, r, b;
        if (!ThreatAnalyzer::parseSquare(w[1], k) || !ThreatAnalyzer::parseSquare(w[2], r) ||
            !ThreatAnalyzer::parseSquare(w[3], b))
            return error(stats, 422, "клетка задаётся буквой a-h и цифрой 1-8");
        ThreatAnalyzer a(k, r, b);
        if (!a.isValid()) return error(stats, 422, "фигуры не могут стоять на одной клетке");
        return {std::string("OK ") + threatName(a.threat()) + " " + a.describe()};
    }
    return error(stats, 400, "неизвестная команда " + w[0]);
}

}  // namespace chess::proto
