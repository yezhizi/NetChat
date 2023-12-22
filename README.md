# NetChat

Simple Server for a Chat App.

## Quick Start

1. Install the toolchain

```sh
apt install build-essential ninja-build cmake
```

2. Intall dependencies

```sh
apt install libprotobuf-dev protobuf-compiler
```

Currently Protobuf is the only dependency requires manual installation. Others will be available when configuring the CMake.  

3. Configure CMake and Build 

```sh
git clone https://github.com/yezhizi/NetChat.git
mkdir -p NetChat/build
cd NetChat/build
cmake -GNinja ..
cmake --build .
```

Configuring CMake will automatically download and install the dependencies. Please make sure you have a working Internet connection.

The binary will be located at `NetChat/bin/NetDesign2-Server`.

4. Run the binary

```sh
cd ..
./bin/NetDesign2-Server
```
