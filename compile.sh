#!/bin/bash

if [ "$#" -eq 1 ] && [ "$1" == "test" ]; then
    BIN="./build/test"
    ENTRYPOINT="tests/test.cpp"
else
    BIN="./build/main"
    ENTRYPOINT="main.cpp"
fi

FLAGS="-g -ggdb -pg -O0 -Wall"
# FLAGS="-O3 -ffast-math"
FLAGS+=" -march=native -masm=intel"
FLAGS+=" -finstrument-functions" # -fsanitize=address"
# SRC+=" gf_profiling.c"
DEFINES="-DADS_DEBUG" # -DADS_SSE"
LINKS="-lc"
LINKSDIR=
INCLUDES="-I. -I./src/"
SRC="./src/*/*.cpp"
CXX=clang++

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
    DEFINES="-DADS_LINUX ${DEFINES}"
    if [[ $XDG_SESSION_TYPE == "x11" ]]; then
        DEFINES="-DADS_X11 ${DEFINES}"
        LINKS="-lX11 ${LINKS}"
    elif [[ $XDG_SESSION_TYPE == "wayland" ]]; then
        DEFINES="-DADS_WAYLAND ${DEFINES}"
        echo "WAYLAND NOT SUPPORTED"
        exit -1
    fi
fi

if [ $BIN ]; then
    CMD="$CXX -o $BIN $ENTRYPOINT $SRC $FLAGS $INCLUDES $LINKS $DEFINES"
else
    FLAGS+=" -S"
    CMD="$CXX $ENTRYPOINT $SRC $FLAGS $INCLUDES $LINKS $DEFINES"
fi

echo "Compiling: "
echo "   $CMD"
SECONDS=0
eval $CMD
SUCCESS=$?
duration=$SECONDS

if [ $SUCCESS -eq 0 ]; then
    echo "Compilation success: $((duration / 60)) min $((duration % 60)) sec elapsed."
    if [ $BIN ]; then
        echo "running $BIN"
        time ./$BIN
    fi
fi
