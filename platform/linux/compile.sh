#!/bin/bash

BIN="./build/main"

FLAGS="-g -fsanitize=address"
DEFINES=
LINKS=
LINKSDIR=
INCLUDES="-I."
ENTRYPOINT="x11xlib.cpp"
WARNINGS="-Wall"
CXX=clang++

if [[ -f $BIN ]]; then
    echo "Removing existing $BIN"
    rm -r $(dirname -- $BIN)
fi
if [[ ! -d "$(dirname -- $BIN)" ]]; then
    mkdir "$(dirname -- $BIN)"
fi

if [[ $XDG_SESSION_TYPE == "x11" ]]; then
    DEFINES="-D_X11 ${DEFINES}"
    LINKS="-lX11 ${LINKS}"
elif [[  $XDG_SESSION_TYPE == "wayland"  ]]; then
    DEFINES="-D_WAYLAND ${DEFINES}"
fi

CMD="$CXX -o $BIN $ENTRYPOINT $FLAGS $INCLUDES $WARNINGS $LINKS"
echo "Compiling: "
echo "    $CMD"
SECONDS=0
eval $CMD
duration=$SECONDS
SUCCESS=$?

if [ $SUCCESS -eq 0 ]; then
    echo "$(date +'%Y-%m-%d %H:%M:%S') Compilation success: $((duration / 60)) min $((duration % 60)) sec elapsed."
    echo "$(date +'%Y-%m-%d %H:%M:%S') running $BIN "
    echo
    ./$BIN
fi
