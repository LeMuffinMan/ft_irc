#!/bin/bash

PORT=6667
SERVER_BIN="./ircserv"
SERVER_LOG=".server_output.log"
RUST_TEST_DIR="tests/irc_tester"
PASSWORD=""

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

if [[ ! -x "$SERVER_BIN" ]]; then
    echo -e "${RED}Error: $SERVER_BIN not found or not executable.${NC}"
    exit 1
fi

rm -f "$SERVER_LOG"

echo "Launching server on port $PORT..."
$SERVER_BIN "$PORT" "$PASSWORD" > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!

sleep 0.5

if ! ps -p $SERVER_PID > /dev/null; then
    echo -e "${RED}Server failed to start.${NC}"
    exit 1
fi
echo -e "${GREEN}Server launched${NC} PID=$SERVER_PID"

echo -e "---- Rust tester ----"
(
    cd "$RUST_TEST_DIR" || exit 1
    cargo run
)
RUST_EXIT=$?

# if [[ $RUST_EXIT -eq 0 ]]; then
#     echo -e "${GREEN}Rust tests finished successfully.${NC}"
# else
#     echo -e "${RED}Rust tests failed (exit code $RUST_EXIT).${NC}"
# fi

# --- Arrêter le serveur ---
echo "> Stopping server..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null
echo "> Server stopped."

# --- Afficher le log du serveur si besoin ---
echo
echo "=== Server log ==="
cat "$SERVER_LOG"
