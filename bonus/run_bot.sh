#!/bin/bash


kill_kitties() {
    echo "Nettoyage en cours..."
    
    pkill -f "kitty --title \"IRC Server\"" 2>/dev/null || true
    pkill -f "kitty --title \"IRC Client\"" 2>/dev/null || true
    pkill -f "kitty --title \"Bots\"" 2>/dev/null || true
    
    sleep 1
}
trap kill_kitties EXIT

if ! command -v kitty &>/dev/null; then
    echo "Error : Kitty not found"
    exit 1
fi

PORT=6667
PASSWORD="password"
NICKNAME="Player"
USERNAME="Player"
KITTY_PID_SERV=0
KITTY_PID_PLAYER=0
KITTY_PID_BOTS=0

kitty --title "IRC Server" --hold bash -c "make && ./ircserv 6667 password" &
KITTY_PID_SERV=$!
echo "Serveur IRC lancé dans Kitty (PID: $KITTY_PID_SERV)"

sleep 2

kitty --title "Bots" --hold bash -c "cd bonus/bot && cargo run" &
KITTY_PID_BOTS=$!
echo "Bots lancés dans Kitty (PID: $KITTY_PID_BOTS)"

sleep 2

kitty --title "IRC Client" --hold sh -c "
    echo 'Connexion au serveur IRC sur port $PORT...'
    {
        echo 'PASS $PASSWORD'
        echo 'NICK $NICKNAME'
        echo 'USER $USERNAME 0 0 0 0'
        cat
    } | nc -C 127.0.0.1 $PORT
" &
KITTY_PID_PLAYER=$!

