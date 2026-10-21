#!/usr/bin/env bash
# Интеграционный тест: запуск сервера, обращения клиента и netcat, параллельные клиенты.
set -u
cd "$(dirname "$0")/.."
PORT=${PORT:-5099}
SERVER=server/chess_server
CLIENT=client/chess_client
LOG=tests/server.log
pass=0; fail=0

check() {   # check "описание" "ожидаемое начало ответа" команда...
  local name=$1 expected=$2; shift 2
  local got; got=$("$@" 2>&1 | head -1)
  if [[ "$got" == "$expected"* ]]; then ((pass++)); echo "OK   $name: $got"
  else ((fail++)); echo "FAIL $name: $got (ожидалось: $expected)"; fi
}

$SERVER -a 127.0.0.1 -p "$PORT" > "$LOG" 2>&1 &
SPID=$!
trap 'kill $SPID 2>/dev/null' EXIT
for _ in $(seq 50); do (exec 3<>/dev/tcp/127.0.0.1/$PORT) 2>/dev/null && break; sleep 0.1; done

check "ладья"            "OK ROOK"   $CLIENT -p "$PORT" e1 e8 a1
check "слон"             "OK BISHOP" $CLIENT -p "$PORT" e1 a8 h4
check "двойной шах"      "OK BOTH"   $CLIENT -p "$PORT" e1 e8 h4
check "перекрытие"       "OK NONE"   $CLIENT -p "$PORT" e1 e8 e4
check "клетка вне доски" "ERR 422"   $CLIENT -p "$PORT" z9 a1 b2
check "команда PING"     "PONG"      $CLIENT -p "$PORT" -c PING
check "неизвестная"      "ERR 400"   $CLIENT -p "$PORT" -c "FOO"
check "netcat"           "OK ROOK"   bash -c "printf 'CHECK e1 e8 a1\nQUIT\n' | nc -q 1 127.0.0.1 $PORT"
check "нет сервера"      "Ошибка: не удалось подключиться" $CLIENT -p 1 e1 e8 a1

# 20 клиентов одновременно
for i in $(seq 20); do $CLIENT -p "$PORT" e1 e8 h4 > "tests/par_$i.out" & done; wait $(jobs -p | grep -v "^$SPID$") 2>/dev/null
ok=$(grep -l "^OK BOTH" tests/par_*.out | wc -l); rm -f tests/par_*.out
if [[ $ok -eq 20 ]]; then ((pass++)); echo "OK   20 параллельных клиентов: $ok ответов"
else ((fail++)); echo "FAIL 20 параллельных клиентов: $ok ответов"; fi

check "статистика"       "OK requests=" $CLIENT -p "$PORT" -c STATS
echo "Пройдено: $pass, ошибок: $fail (журнал сервера: $LOG)"
[[ $fail -eq 0 ]]
