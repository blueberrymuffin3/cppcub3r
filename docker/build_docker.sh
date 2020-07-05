set -e

if [ -f /.dockerenv ]; then
    mkdir -p src/builddir
    cd src/builddir

    CC=arm-linux-gnueabi-gcc CXX=arm-linux-gnueabi-g++ cmake .. -GNinja
    ninja
else
    echo "ERROR: Not in a docker container"
    exit 1
fi
