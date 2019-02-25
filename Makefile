# define some make variables
PLEORA_ROOT = /opt/pleora/ebus_sdk/Ubuntu-x86_64/
PLEORA_HEADERS = $(PLEORA_ROOT)include
PLEORA_LIB = $(PLEORA_ROOT)lib
GENICAM_LIB = $(PLEORA_ROOT)lib/genicam/bin/Linux64_x64
WRAPPER_DIR = ebus-wrapper
WRAPPER_H = wrapper.h
WRAPPER_CPP = wrapper.cpp


# make the ebus wrapper library
build: $(WRAPPER_DIR)/$(WRAPPER_CPP) $(WRAPPER_DIR)/$(WRAPPER_H)
	clang -g -c $(WRAPPER_DIR)/$(WRAPPER_CPP) -o wrapper.o -I./$(WRAPPER_DIR) -I${PLEORA_HEADERS} -m64 -fPIC
	clang --shared wrapper.o -o libwrapper.so

# make a main program using the wrapper library
main: examples/main.c $(WRAPPER_DIR)/$(WRAPPER_CPP) $(WRAPPER_DIR)/$(WRAPPER_H)
	cc -g examples/main.c -I. -L. -lwrapper -I${PLEORA_HEADERS} -L${PLEORA_LIB} -L${GENICAM_LIB} \
		-lPvSystem -lPvDevice -lPvBuffer -lPvBase -lPvStream -lPtUtilsLib  \
		-lPvGenICam -lstdc++ -fPIC -o main

# build the rust wrapper
rust:
	cargo build --release

# clean up the former runs
clean:
	cargo clean
	rm ./libwrapper.so ./wrapper.o
