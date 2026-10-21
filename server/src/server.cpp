// server.cpp — TCP-сервер «шахматного» сервиса (задача 2.25).
// Принимает соединения, на каждое — отдельный поток; читает команды построчно
// и отвечает по протоколу CTP/1.0 (lib/protocol.h).
//
// Запуск: chess_server [-a адрес] [-p порт]      (по умолчанию 0.0.0.0:5025)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <ctime>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include "protocol.h"

namespace {

std::mutex g_log_mutex;
std::atomic<bool> g_running{true};
int g_listen_fd = -1;
chess::proto::Stats g_stats;

// Журнал: время, адрес клиента, сообщение.
void log(const std::string& peer, const std::string& msg) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    char ts[32];
    std::time_t now = std::time(nullptr);
    std::strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    std::cout << ts << "  " << peer << "  " << msg << std::endl;
}

void on_signal(int) {
    g_running = false;
    if (g_listen_fd >= 0) shutdown(g_listen_fd, SHUT_RDWR);   // прервать accept()
}

bool send_all(int fd, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = send(fd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (n <= 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

// Обслуживание одного клиента. std::thread хранит свою копию аргумента,
// поэтому ссылка остаётся действительной всё время работы потока.
void serve(int fd, const std::string& peer) {
    long active = ++g_stats.clients;
    log(peer, "подключился (активных клиентов: " + std::to_string(active) + ")");
    std::string buffer;
    char chunk[1024];
    bool open = true;
    while (open && g_running) {
        ssize_t n = recv(fd, chunk, sizeof chunk, 0);
        if (n <= 0) break;                                // клиент закрыл соединение
        buffer.append(chunk, static_cast<size_t>(n));
        if (buffer.size() > 4096) {                       // защита от слишком длинных строк
            send_all(fd, "ERR 413 слишком длинная строка\n");
            break;
        }
        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            if (!line.empty() && line.back() == '\r') line.pop_back();   // telnet/Windows
            chess::proto::Reply reply = chess::proto::handle(line, g_stats);
            log(peer, "<- " + line);
            log(peer, "-> " + reply.text);
            if (!send_all(fd, reply.text + "\n") || reply.close) { open = false; break; }
        }
    }
    close(fd);
    active = --g_stats.clients;
    log(peer, "отключился (активных клиентов: " + std::to_string(active) + ")");
}

}  // namespace

// Сигнатура main задана стандартом C++, поэтому замечание подавлено.
// cppcheck-suppress constParameter
int main(int argc, char* argv[]) {
    std::string addr = "0.0.0.0";
    int port = chess::proto::kDefaultPort;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-p" || a == "--port") && i + 1 < argc) port = std::stoi(argv[++i]);
        else if ((a == "-a" || a == "--address") && i + 1 < argc) addr = argv[++i];
        else {
            std::cerr << "Использование: " << argv[0] << " [-a адрес] [-p порт]\n";
            return 2;
        }
    }

    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    g_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_listen_fd < 0) { std::perror("socket"); return 1; }
    int yes = 1;
    setsockopt(g_listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, addr.c_str(), &sa.sin_addr) != 1) {
        std::cerr << "Некорректный адрес: " << addr << "\n";
        return 2;
    }
    if (bind(g_listen_fd, reinterpret_cast<sockaddr*>(&sa), sizeof sa) < 0) { std::perror("bind"); return 1; }
    if (listen(g_listen_fd, 16) < 0) { std::perror("listen"); return 1; }
    log("server", std::string("слушает ") + addr + ":" + std::to_string(port) + " ("
                  + chess::proto::kProtocolVersion + ")");

    while (g_running) {
        sockaddr_in ca{};
        socklen_t len = sizeof ca;
        int fd = accept(g_listen_fd, reinterpret_cast<sockaddr*>(&ca), &len);
        if (fd < 0) {
            if (!g_running) break;
            if (errno == EINTR) continue;
            std::perror("accept");
            continue;
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ca.sin_addr, ip, sizeof ip);
        std::string peer = std::string(ip) + ":" + std::to_string(ntohs(ca.sin_port));
        std::thread(serve, fd, peer).detach();
    }
    close(g_listen_fd);
    log("server", "остановлен; обработано запросов: " + std::to_string(g_stats.requests.load()));
    return 0;
}
