#!/bin/bash

# This script execute the tested irc server with a $PASSWORD and on a $PORT
# Then, it executes the tester with provided arguments
# Ultimately, it prints logs from .server.log file
#
# By default, using 'make test' will execute each unit test asynchronously, then NUM_CLIENT 
# connection tests, and more diversified tests, with a NUM_CLIENT=1000
#
# Using 'make test NUM_CLIENT=500' will execute maximum 500 clients at each wave
#
# Using STRESS=1 launch only stress tests
# Using BEH=1 launch only behaviors tests
#
# Having terminator terminal install allow to monitor server output in real time
# using : 
# make test LOG=1
#
SERVER="./ircserv"
PORT=6667
PASS="password"
CLIENT_DIR="tests/irc_tester"
VALGRIND="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() {
    echo -e "\n${YELLOW}====$*====${NC}"
}

kill_server() {
    if [[ -n "$SERVER_PID" ]]; then
        kill "$SERVER_PID" 2>/dev/null || true
    fi
}
trap kill_server EXIT

ulimit -n $(ulimit -Hn)
echo -e "${YELLOW}ulimit = $(ulimit -n)${NC}"

log "Starting server on port $PORT..."
if [ "$2" == 1 ]; then
    $VALGRIND $SERVER "$PORT" "$PASS" > .server.log 2>&1 &
else
    $SERVER "$PORT" "$PASS" > .server.log 2>&1 &
fi

SERVER_PID=$!

sleep 1

if kill -0 "$SERVER_PID" 2>/dev/null; then
echo -e "${GREEN}Server PID: $SERVER_PID${NC}"
else
    echo "Server failed to start"
    echo "--- Server log ---"
    cat .server.log
    exit 1
fi

sleep 1

log "Building Rust tester..."
(
    cd "$CLIENT_DIR"
    cargo build --release
) || exit 1

TERMINATOR_PID=0
if command -v terminator &>/dev/null && [ "$4" = 1 ]; then
    terminator --title "IRC Server Log" -e "tail -f .server.log" 2> /dev/null &
    TERMINATOR_PID=$!
fi

START_TIME=$(date +%s)
log "Starting Rust tester..."

"$CLIENT_DIR/target/release/irc_tester" "$1" "$3" "$5"

echo "Killing server PID: $SERVER_PID"
kill $SERVER_PID

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))
EVENTS=$(cat .server.log | wc -l)

ulimit -n 1024

log "============== Server log =============="
# verbose mode
ERRORS_TOTAL=$(grep -c "ERROR" .server.log)
ERRORS_UNIQUE=$(grep "ERROR" .server.log | sort | uniq | wc -l)
PINGS=$(grep -c "PING" .server.log)
PONGS=$(grep -c "PONG" .server.log)
CONNECTIONS=$(grep -c "New client" .server.log)
REGISTRATIONS=$(grep -c "Welcome to the Internet Relay Network" .server.log)
TIMEDOUT=$(grep -c "timed out" .server.log)

EVENTS=${EVENTS:-0}
ELAPSED=${ELAPSED:-1}
EVENTS_PER_SECS=$(( EVENTS / ELAPSED ))

printf "%-25s : %5s\n" "Time elapsed": "${ELAPSED}s"
printf "%-25s : %5d (%d/s)\n" "Events" "${EVENTS}" "$EVENTS_PER_SECS"
echo
printf "%-25s : %5d (unique: %d)\n" "Errors" "$ERRORS_TOTAL" "$ERRORS_UNIQUE"
if [[ $errors_unique -gt 0 ]]; then
    grep "ERROR" .server.log |  awk '{$1=""; sub(/^ /, ""); print}' | sort | uniq -c | while read count msg; do
        printf "%s x %5d\n" "$MSG" "$COUNT" 
    done
fi
printf "%-25s : %5d (%d/s)\n" "Connections" "$CONNECTIONS" "$((CONNECTIONS / ELAPSED))"
printf "%-25s : %5d (%d/s)\n" "Registrations" "$REGISTRATIONS" "$((REGISTRATIONS / ELAPSED))"
printf "%-25s : %5d (%d/s)\n" "Timed out" "$TIMEDOUT" "$((TIMEDOUT / ELAPSED))"
echo

if [[ -n "$TERMINATOR_PID" ]]; then
    echo "Killing log watcher (terminator) PID: $TERMINATOR_PID"
    kill "$TERMINATOR_PID" 2>/dev/null || true
fi
