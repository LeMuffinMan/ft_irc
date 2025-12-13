#!/bin/bash

if ! command -v terminator &>/dev/null; then
    echo "Error : Terminator not found"
    exit 1
fi

kill_terminals() {
    echo "Erasing..."
    
    kill -9 $TERMINATOR_PID_SERV 2>/dev/null || true
    kill -9 $TERMINATOR_PID_PLAYER 2>/dev/null || true
    kill -9 $TERMINATOR_PID_BOTS 2>/dev/null || true

    sleep 1
}

# trap kill_terminals EXIT

PORT=6667
PASSWORD="password"
NICKNAME="Player"
USERNAME="Player"
TERMINATOR_PID_SERV=0
TERMINATOR_PID_PLAYER=0
TERMINATOR_PID_BOTS=0

terminator --title "IRC Server" -e 'bash -c "make && ./ircserv 6667 password"' 2>/dev/null &
TERMINATOR_PID_SERV=$!
echo "IRC Server started in Terminator (PID: $TERMINATOR_PID_SERV)"

sleep 2

terminator --title "Bots" -e 'bash -c "cd bonus/bot && cargo run"' 2>/dev/null &
TERMINATOR_PID_BOTS=$!
echo "Bots started in Terminator (PID: $TERMINATOR_PID_BOTS)"

sleep 2

TEMP_SCRIPT=$(mktemp)
cat > "$TEMP_SCRIPT" << EOF
#!/bin/sh
echo 'Connexion au serveur IRC sur port $PORT...'
{
    echo 'PASS $PASSWORD'
    echo 'NICK $NICKNAME'
    echo 'USER $USERNAME 0 0 0 0'
    cat
} | nc -C 127.0.0.1 $PORT
EOF

chmod +x "$TEMP_SCRIPT"
terminator --title="IRC Client" -e "$TEMP_SCRIPT" &
TERMINATOR_PID_PLAYER=$!
