// client.cpp — сетевой клиент «шахматного» сервиса (задача 2.25).
//
//   chess_client [-H хост] [-p порт] e1 e8 h4        — одна позиция
//   chess_client [-H хост] [-p порт] -c "STATS"      — произвольная команда протокола
//   chess_client [-H хост] [-p порт] -f positions.txt — пакет позиций из файла
//   chess_client [-H хост] [-p порт]                 — интерактивный режим
#ifdef _WIN32                     // Windows: Winsock2 (сборка MinGW: -lws2_32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
using ssize_t = long long;
#define close closesocket
#define MSG_NOSIGNAL 0
#else                             // Linux, macOS: сокеты POSIX
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include <chrono>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

class Connection {
public:
    Connection(const std::string& host, const std::string& port) {
        addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_UNSPEC;          // IPv4 или IPv6
        hints.ai_socktype = SOCK_STREAM;      // TCP
        int rc = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
        if (rc != 0) throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(rc));
        for (addrinfo* p = res; p; p = p->ai_next) {
            fd_ = static_cast<int>(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
            if (fd_ < 0) continue;
#ifdef _WIN32
            DWORD tv = 5000;                  // таймаут ответа 5 с
            setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof tv);
#else
            timeval tv{5, 0};                 // таймаут ответа 5 с
            setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
#endif
            if (connect(fd_, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) break;
            close(fd_);
            fd_ = -1;
        }
        freeaddrinfo(res);
        if (fd_ < 0)
            throw std::runtime_error("не удалось подключиться к " + host + ":" + port +
                                     " (" + std::strerror(errno) + ")");
    }
    ~Connection() { if (fd_ >= 0) close(fd_); }

    // Отправить строку и получить строку ответа.
    std::string request(const std::string& line) {
        std::string data = line + "\n";
        if (send(fd_, data.data(), static_cast<int>(data.size()), MSG_NOSIGNAL) != static_cast<ssize_t>(data.size()))
            throw std::runtime_error("ошибка отправки");
        size_t pos;
        while ((pos = buffer_.find('\n')) == std::string::npos) {
            char chunk[1024];
            ssize_t n = recv(fd_, chunk, sizeof chunk, 0);
            if (n <= 0) throw std::runtime_error("сервер закрыл соединение или не ответил");
            buffer_.append(chunk, static_cast<size_t>(n));
        }
        std::string reply = buffer_.substr(0, pos);
        buffer_.erase(0, pos + 1);
        return reply;
    }

private:
    int fd_ = -1;
    std::string buffer_;
};

// Запрос с замером времени; печать ответа. Возвращает true, если ответ не ERR.
bool ask(Connection& c, const std::string& line, bool verbose) {
    auto t0 = std::chrono::steady_clock::now();
    std::string reply = c.request(line);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::steady_clock::now() - t0).count();
    if (verbose) std::cout << "> " << line << "\n";
    std::cout << reply;
    if (verbose) std::cout << "   [" << us / 1000.0 << " мс]";
    std::cout << "\n";
    return reply.rfind("ERR", 0) != 0;
}

void usage(const char* p) {
    std::cerr << "Использование:\n"
              << "  " << p << " [-H хост] [-p порт] КОРОЛЬ ЛАДЬЯ СЛОН\n"
              << "  " << p << " [-H хост] [-p порт] -c КОМАНДА\n"
              << "  " << p << " [-H хост] [-p порт] -f ФАЙЛ\n"
              << "  " << p << " [-H хост] [-p порт]          (интерактивно)\n";
}

}  // namespace

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { std::cerr << "WSAStartup\n"; return 3; }
#endif
    std::string host = "127.0.0.1", port = "5025", command, file;
    std::vector<std::string> squares;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-H" || a == "--host") && i + 1 < argc) host = argv[++i];
        else if ((a == "-p" || a == "--port") && i + 1 < argc) port = argv[++i];
        else if (a == "-c" && i + 1 < argc) command = argv[++i];
        else if (a == "-f" && i + 1 < argc) file = argv[++i];
        else if (a == "-h" || a == "--help") { usage(argv[0]); return 0; }
        else if (!a.empty() && a[0] != '-') squares.push_back(a);
        else { usage(argv[0]); return 2; }
    }
    if (!squares.empty() && squares.size() != 3) { usage(argv[0]); return 2; }

    try {
        Connection c(host, port);
        if (squares.size() == 3)
            return ask(c, "CHECK " + squares[0] + " " + squares[1] + " " + squares[2], false) ? 0 : 1;
        if (!command.empty())
            return ask(c, command, false) ? 0 : 1;
        if (!file.empty()) {                      // пакетный режим
            std::ifstream in(file);
            if (!in) { std::cerr << "Не открыть файл " << file << "\n"; return 2; }
            int ok = 0, total = 0;
            for (std::string line; std::getline(in, line);) {
                if (line.empty() || line[0] == '#') continue;
                ++total;
                ok += ask(c, "CHECK " + line, true);
            }
            ask(c, "STATS", true);
            c.request("QUIT");
            std::cout << "Отправлено позиций: " << total << ", успешных ответов: " << ok << "\n";
            return 0;
        }
        std::cout << "Подключено к " << host << ":" << port << ". Команды: CHECK e1 e8 h4, "
                  << "STATS, PING, HELP, QUIT\n";
        for (std::string line; std::cout << "chess> " << std::flush, std::getline(std::cin, line);) {
            if (line.empty()) continue;
            ask(c, line, false);
            if (line == "QUIT" || line == "quit") break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 3;
    }
    return 0;
}
