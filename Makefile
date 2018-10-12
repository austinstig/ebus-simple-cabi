# define some make variables
PLEORA_ROOT = $(PUREGEV_ROOT)
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
main: main.c $(WRAPPER_DIR)/$(WRAPPER_CPP) $(WRAPPER_DIR)/$(WRAPPER_H)
	cc -g main.c -I. -L. -lwrapper -I${PLEORA_HEADERS} -L${PLEORA_LIB} -L${GENICAM_LIB} \
		-lPvSystem -lPvDevice -lPvBuffer -lPvBase -lPvStream -lPtUtilsLib  \
		-lPvGenICam -lstdc++ -fPIC -o main

rust:
	cd ./ebus && cargo build --release && cd ..

upload: 
	cp ./libwrapper.so ~/Downloads/libwrapper.so
	cp ./main ~/Downloads/main
	cp ./ebus/target/release/ebus-bin ~/Downloads/ebus-bin

clean:
	cd ./ebus && cargo clean && cd ..
	rm ./main libwrapper.so wrapper.o
	rm ~/Downloads/main
	rm ~/Downloads/ebus-bin
	rm ~/Downloads/libwrapper.so