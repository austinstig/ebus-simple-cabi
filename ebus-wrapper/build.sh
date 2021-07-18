#!/bin/bash
g++ -c -I/opt/pleora/ebus_sdk/Ubuntu-18.04-x86_64/include -I.  -O3 -D_UNIX_ -D_LINUX_ -DQT_GUI_LIB -fPIC -std=c++11 -o wrapper.o wrapper.cpp
#g++ -shared -fPIC -o wrapper.so wrapper.o
g++ -shared wrapper.o -o wrapper.so -L/opt/pleora/ebus_sdk/Ubuntu-18.04-x86_64/lib -lPvAppUtils -lPtConvertersLib -lPvBase -lPvBuffer -lPvGenICam -lPvSystem -lPvStream -lPvDevice -lPvTransmitter -lPvVirtualDevice -lPvPersistence -lPvSerial -lPvCameraBridge -lPvGUI -lSimpleImagingLib -L/opt/pleora/ebus_sdk/Ubuntu-18.04-x86_64/lib/genicam/bin/Linux64_x64
#sudo cp -f wrapper.so /usr/local/lib/libEBUSWrapper.so
echo "/home/myworld/Github/ebus-simple-cabi/ebus-wrapper" > /etc/ld.so.conf.d/ebus_wrapper.conf
ldconfig
