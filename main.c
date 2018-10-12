#include <stdio.h>
#include <stdlib.h>
#include "ebus-wrapper/wrapper.h"

// MAIN
int main() {

    // find devices
    const char*  output = w_find(300);

    // make a state variables
    EBUSState* state = mkstate();

    if (output != NULL) {
        printf("device: %s\n", output);

        // connect to a device
        w_connect(state, output, 200, 3);
        w_configure(state, "PixelFormat\0", "Mono16\0");

        long tf = w_tick_frequency(state);
        printf("tick frequency: %li\n", tf);

        // start the acquisition
        w_begin_streaming(state);

        // open output file
        //FILE* outfile = fopen("output.dat", "wb");
        //if (outfile != NULL) {

            // acquire a few frames
            unsigned char* bytes;
            int num = 655360;
            for (int i = 0; i<10*60*30; i++) {
                bytes = (unsigned char*) malloc(num);
                long ts = w_acquire(state,bytes, 655360, 1000);
                //fwrite(bytes, 1, 655360, outfile);
                printf("ts: %li\n", ts);
            }
        //}
        //fclose(outfile);

    } else {
        printf("no devices found!\n");
    }

    // shutdown the acquisition
    w_shutdown(state);

    // end the program
    printf("program complete\n");
    return 0;
}
