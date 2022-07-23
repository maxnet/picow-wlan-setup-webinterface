# picow-wlan-setup-webinterface

![screenshot](https://github.com/maxnet/picow-wlan-setup-webinterface/raw/master/screenshots/animated-gif.gif)

Proof of concept that lets the Pico-W open up a temporary access point, with a webinterface to configure the real WLAN settings.

## Build dependencies

On Debian:

```
sudo apt install git build-essential cmake gcc-arm-none-eabi
```

Your Linux distribution does need to provide a recent CMake (3.13+).
If not, compile [CMake from source](https://cmake.org/download/#latest) first.

## Build instructions

```
git clone --depth 1 https://github.com/maxnet/picow-wlan-setup-webinterface
cd picow-wlan-setup-webinterface
git submodule update --init
cd pico-sdk
git submodule update --init
cd ..
mkdir -p build
cd build
cmake .. -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=MinSizeRel
make
```

Copy the resulting picow-wlan-setup-webinterface.uf2 file to the Pico mass storage device manually.
Webserver will be available at ANY DNS name (e.g. http://www.picow/ )
But easiest on Android is to use the "manage router" button in the WLAN settings screen.
