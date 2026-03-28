#!/bin/bash

FILE=src/libs/ads_string.c
gcc -I./src/ -D_GNU_SOURCE -std=c99 -Wall -Wextra -Werror -Wpedantic -c $FILE -o /tmp/a.o
echo "DONE"

#
# set -e
#
# # C libs
# CFLAGS="-std=c99 -Wall -Wextra -Werror -Wpedantic"
# CPPFLAGS="-std=c++11 -Wall -Wextra -Werror -Wpedantic"
#
# DEFINES="-DADS_LINUX -DADS_X11 -D_GNU_SOURCE"
# INCLUDES="-I./src/ -I./"
# LINKS="-lX11"
#
# BUILD_DIR="./build"
#
# mkdir -p "$BUILD_DIR"
#
# function compile_file () {
#     local filepath="$1"              # e.g. libs/foo.cpp
#     local src_path="$filepath"   # actual path
#     local obj_path="$BUILD_DIR/${filepath#src/}.o"
#     echo $src_path
#
#     mkdir -p "$(dirname "$obj_path")"
#
#     local extension="${filepath##*.}"
#     local COMPILER=""
#     local FLAGS=""
#
#     if [[ "$extension" == "c" ]]; then
#         COMPILER=clang
#         FLAGS="$CFLAGS"
#     elif [[ "$extension" == "cpp" ]]; then
#         COMPILER=clang++
#         FLAGS="$CPPFLAGS"
#     else
#         echo "Skipping unsupported file: $filepath"
#         return
#     fi
#
#     # Only compile if needed
#     # if [[ -f "$obj_path" && "$obj_path" -nt "$src_path" ]]; then
#     #     echo "Up to date: $filepath"
#     #     return
#     # fi
#
#     echo "Compiling: $filepath"
#     "$COMPILER" $FLAGS $DEFINES $INCLUDES -c "$src_path" -o "$obj_path"
# }
#
# echo "---- Compiling libraries ----"
#
# # Safer than ls (handles spaces properly)
# find src/* -type f \( -name "*.c" -o -name "*.cpp" \) | while read -r file; do
#     compile_file "$file"
# done
#
# # rm -r "$BUILD_DIR"/platform/linux/
#
#
# find "$BUILD_DIR" -name "*.o" -print0 | xargs -0 clang++ $LINKS -o "$BUILD_DIR/app"
# echo "Build complete: $BUILD_DIR/app"


