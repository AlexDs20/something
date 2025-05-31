#!/bin/bash

BIN="./build/main"

FLAGS="-g -ggdb -pg -O0 -Wall" # -finstrument-functions" # -fsanitize=address"
DEFINES=-D_DEBUG
LINKS="-lc"
LINKSDIR=
INCLUDES="-I."
ENTRYPOINT="main.cpp"
SRC="./*/*.cpp"
CXX=g++

function echo () {
    builtin echo "[$(date +'%Y-%m-%d %H:%M:%S.%3N')] $@"
}

if [[ -f $BIN ]]; then
    echo "Removing existing $BIN"
    rm -r $(dirname -- $BIN)
fi
if [[ ! -d "$(dirname -- $BIN)" ]]; then
    mkdir "$(dirname -- $BIN)"
fi

# Handle platform and define needed stuff
if [[ $(uname) == "Linux" ]]; then
    DEFINES="-D_LINUX ${DEFINES}"
    if [[ $XDG_SESSION_TYPE == "x11" ]]; then
        DEFINES="-D_X11 ${DEFINES}"
        LINKS="-lX11 ${LINKS}"
    elif [[ $XDG_SESSION_TYPE == "wayland" ]]; then
        DEFINES="-D_WAYLAND ${DEFINES}"
        echo "WAYLAND NOT SUPPORTED"
        exit -1
    fi
fi

CMD="$CXX -o $BIN $ENTRYPOINT $SRC $FLAGS $INCLUDES $LINKS $DEFINES"
echo "Compiling: "
echo "   $CMD"
SECONDS=0
eval $CMD
SUCCESS=$?
duration=$SECONDS

if [ $SUCCESS -eq 0 ]; then
    echo "Compilation success: $((duration / 60)) min $((duration % 60)) sec elapsed."
    echo "running $BIN"
    ./$BIN
fi
