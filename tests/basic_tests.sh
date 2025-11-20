#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' 


PORT=${1:-6667}
SERVER_BIN="./ircserv"
SERVER_LOG=".server_output.log"
HOST=127.0.0.1

echo "> Port: $PORT"
echo "> Serveur: $SERVER_BIN"
echo "> Log: $SERVER_LOG"
echo

if [[ ! -x "$SERVER_BIN" ]]; then
    echo -e "${RED}Error: $SERVER_BIN not found.${NC}"
    exit 1
fi

pkill ircserv
rm -f "$SERVER_LOG"
echo -n "Launch server: "

$SERVER_BIN "$PORT" testpassword > "$SERVER_LOG" 2>&1 &

SERVER_PID=$!
sleep 0.5

if ! ps -p $SERVER_PID > /dev/null; then
    echo -e "${RED}KO Server failed to launch, no active process id $SERVER_PID.${NC}"
    exit 1
fi
echo -e "${GREEN}OK${NC} pid = $SERVER_PID"


echo -n "Simple client connexion: "
timeout 1 bash -c "timeout 1 nc $HOST $PORT <<< ''"
if grep -Fq "New client: 5" "$SERVER_LOG"; then
    echo -e " ${GREEN}OK${NC}"
else
    echo -e " ${RED}KO${NC}"
fi


echo -n "Regular msg: "
printf "hello\r\n" | timeout 1 nc $HOST $PORT
if grep -Fq "msg from 5: [hello]" "$SERVER_LOG"; then
    echo -e " ${GREEN}OK${NC}"
else
    echo -e " ${RED}KO${NC}"
fi


echo -n "Multi‑lines: "
printf "a\r\nb\r\nc\r\n" | timeout 1 nc $HOST $PORT
if grep -Fq "msg from 5: [a]" "$SERVER_LOG" && \
   grep -Fq "msg from 5: [b]" "$SERVER_LOG" && \
   grep -Fq "msg from 5: [c]" "$SERVER_LOG"; then
    echo -e " ${GREEN}OK${NC}"
else
    echo -e " ${RED}KO${NC}"
fi


echo -n "Fragmentate: "
(
    printf "par";
    sleep 0.2;
    printf "tial";
    sleep 0.2;
    printf " msg\r\n";
) | timeout 1 nc $HOST $PORT
if grep -Fq "msg from 5: [partial msg]" "$SERVER_LOG"; then
    echo -e " ${GREEN}OK${NC}"
else
    echo -e " ${RED}KO${NC}"
fi


echo "Server shutdown..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo "Server OFF."
echo ".server_output.log:"
echo
cat .server_output.log
echo
