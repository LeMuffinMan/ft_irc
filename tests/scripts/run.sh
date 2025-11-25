#!/bin/bash

SERVER="./ircserv"
PORT=6667
PASS="password"
CLIENT_DIR="tests/irc_tester"

kill_server() {
    if [[ -n "$SERVER_PID" ]]; then
        kill "$SERVER_PID" 2>/dev/null || true
    fi
}
trap kill_server EXIT

echo "Starting server on port $PORT..."
$SERVER "$PORT" "$PASS" > .server.log 2>&1 &
SERVER_PID=$!

sleep 1

if kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "Server PID: $SERVER_PID"
else
    echo "Server failed to start"
    echo "--- Server log ---"
    cat .server.log
    exit 1
fi

sleep 1

echo "Starting Rust tester..."
(
    cd "$CLIENT_DIR"
    cargo run
)

echo "Kill " $SERVER_PID 
kill $SERVER_PID

# echo "--- Server log ---"
# verbose mode
# cat .server.log
