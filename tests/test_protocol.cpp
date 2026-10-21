// test_protocol.cpp — модульные тесты бизнес-логики сервера без сети.
#include <iostream>
#include <string>
#include "protocol.h"

static int g_fail = 0, g_total = 0;

static void expect(const std::string& request, const std::string& expected_prefix, bool close = false) {
    chess::proto::Stats s;
    auto r = chess::proto::handle(request, s);
    ++g_total;
    bool ok = r.text.rfind(expected_prefix, 0) == 0 && r.close == close;
    if (!ok) ++g_fail;
    std::cout << (ok ? "OK   " : "FAIL ") << "\"" << request << "\" -> " << r.text << "\n";
}

int main() {
    expect("CHECK e1 e8 a1", "OK ROOK Королю угрожает ладья с e8.");
    expect("CHECK e1 a8 h4", "OK BISHOP Королю угрожает слон с h4.");
    expect("CHECK e1 e8 h4", "OK BOTH ");
    expect("CHECK e1 b8 h8", "OK NONE Угрозы королю нет.");
    expect("CHECK e1 e8 e4", "OK NONE");                 // слон закрывает ладью
    expect("check E1 E8 A1", "OK ROOK");                 // регистр не важен
    expect("CHECK e1 e8 a1\r", "OK ROOK");               // \r в конце (telnet, Windows) не мешает
    expect("CHECK z9 a1 b2", "ERR 422");
    expect("CHECK e1 e1 a5", "ERR 422");
    expect("CHECK e1 e8", "ERR 400");
    expect("FOO", "ERR 400");
    expect("", "ERR 400");
    expect("PING", "PONG");
    expect("VERSION", "OK chess-threat-server CTP/1.0");
    expect("QUIT", "BYE", true);

    chess::proto::Stats s;                                // счётчики
    chess::proto::handle("PING", s);
    chess::proto::handle("FOO", s);
    auto r = chess::proto::handle("STATS", s);
    ++g_total;
    bool ok = r.text == "OK requests=3 clients=0 errors=1";
    if (!ok) ++g_fail;
    std::cout << (ok ? "OK   " : "FAIL ") << "STATS после PING и FOO -> " << r.text << "\n";

    std::cout << "Пройдено: " << g_total - g_fail << " из " << g_total << "\n";
    return g_fail == 0 ? 0 : 1;
}
