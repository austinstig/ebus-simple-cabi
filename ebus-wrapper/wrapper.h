#ifndef __WRAPPER_H
#define __WRAPPER_H
#include <stdlib.h>
#include <stdbool.h>

// define some structures
typedef struct EBUSState EBUSState;

// c functions
#ifdef __cplusplus
extern "C" {
#endif

    // constructors and allocators
    EBUSState* mkstate();

    // define some methods to link to
    void w_connect(EBUSState* state, const char* info, unsigned int timeout, int nbuffers);
    long w_tick_frequency(EBUSState* state);
    bool w_configure(EBUSState* state, const char* parameter, const char* value);
    void w_begin_streaming(EBUSState* state );
    void w_shutdown(EBUSState* e);

    // enqueue and dequeue capture buffers
    long w_acquire( EBUSState* state, unsigned char* buffer, int buflen, int timeout);

    bool w_is_active(EBUSState* state);
    const char* w_find(unsigned int timeout);

#ifdef __cplusplus
}
#endif
#endif