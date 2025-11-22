#!/bin/bash

SERVER="./ircserv"
PORT=6667
PASS="a"
CLIENT_DIR="tests/irc_tester"

kill_server() {
    echo "Stopping server..."
    if [[ -n "$SERVER_PID" ]]; then
        kill "$SERVER_PID" 2>/dev/null || true
    fi
}
trap kill_server EXIT

echo "Starting server on port $PORT..."
$SERVER "$PORT" "$PASS" &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

sleep 1

echo "Starting Rust client..."
(
    cd "$CLIENT_DIR"
    cargo run
)

echo "Kill " $SERVER_PID 
kill $SERVER_PID
