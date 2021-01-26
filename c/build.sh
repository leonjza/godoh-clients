#!/bin/bash

set -e

CC=clang

if ! [[ "$1" =~ ^(debug|release)$ ]]; then
    echo "Usage: $0 [build type] <domain>"
    echo " eg: $0 release c2.local"
    echo
    echo " Build types can be debug; release"
    exit 1
fi

if [ -z "$2" ]; then
    echo "we need a domain for the agent"
    exit 1
fi

rm -Rf build
mkdir -p build
cd build/

case $1 in
    debug)
        echo "building debug agent for $2"
        cmake ../godoh -DCMAKE_BUILD_TYPE=Debug -DDEBUG=1 -DDOMAIN=$2
        ;;
    release)
        echo "building release agent for $2"
        cmake ../godoh -DCMAKE_BUILD_TYPE=MinSizeRel -DDOMAIN=$2
        ;;
esac

cmake --build .
cd - > /dev/null
echo -n "binary should be in build dir: "
ls -l build/godoh
