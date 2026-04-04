#!/bin/bash
CFLAGS="-std=c99 -Wall -Wextra -Werror -Wpedantic"
CPPFLAGS="-std=c++11 -Wall -Wextra -Werror -Wpedantic"
EXTRAFLAGS="-fsanitize=address,undefined,leak"

DEFINES="-DADS_LINUX -DADS_X11 -D_GNU_SOURCE -DADS_USE_EXTERNAL"
DEFINES+=" -DADS_DEBUG"
INCLUDES="-I./src/ -I./"
LINKS="-lX11"

CCOMP=clang
CPPCOMP=clang++

BUILD_DIR="./build"
EXE="${BUILD_DIR}/main"

mkdir -p "$BUILD_DIR"
rm $EXE

function compile_file () {
    local filepath="$1"
    local src_path="$filepath"
    local obj_path="$BUILD_DIR/${filepath#src/}.o"

    mkdir -p "$(dirname "$obj_path")"

    local extension="${filepath##*.}"
    local COMPILER=""
    local FLAGS=""

    if [[ "$extension" == "c" ]]; then
        COMPILER=$CCOMP
        FLAGS="$CFLAGS"
    elif [[ "$extension" == "cpp" ]]; then
        COMPILER=$CPPCOMP
        FLAGS="$CPPFLAGS"
    else
        echo "Skipping unsupported file: $filepath"
        return
    fi

    # Only compile if needed
    if [[ -f "$obj_path" && "$obj_path" -nt "$src_path" ]]; then
        echo "Up to date: $filepath"
        return
    fi

    echo "Compiling: $filepath"
    "$COMPILER" $FLAGS $DEFINES $INCLUDES -c "$src_path" -o "$obj_path"
    echo "Done: $filepath"
}

find src/* -type f \( -name "*.c" -o -name "*.cpp" \) | while read -r file; do
    compile_file "$file"
done

rm -r "$BUILD_DIR"/platform/*/

echo "Compiling:"
OBJ_FILES=$(find "$BUILD_DIR" -name "*.o")
$CPPCOMP $OBJ_FILES -o $EXE $LINKS
echo "Build complete: $EXE"
