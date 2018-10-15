#include "wrapper.h"
#include <stdio.h>
#include <stdbool.h>

const unsigned BUFFER_COUNT=3; // <--- SET THIS VALUE


#ifdef __cplusplus

#include <cstring>
#include <iostream>
#include <list>
#include <string>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvBuffer.h>
#include <PvSystem.h>
#include <PvSystemEventSink.h>
#include <PvSystemLib.h>
#include <PvBase.h>
#include <PvGenParameterArray.h>

// define some structures
struct EBUSState {
    PvDevice* device;
    PvStream* stream;
    std::list<PvBuffer*>* buffers;

    ~EBUSState() {

        // delete existing buffers
        if (buffers != NULL ) {
            if (buffers->size() > 0) {
                // free all buffers in the stream
                std::list<PvBuffer*>::iterator it = (*buffers).begin();
                while( it != (*buffers).end()) {
                    delete *it;
                    it++;
                }
                // Clear the buffer list 
                (*buffers).clear();
            }
            // set list to NULL
            buffers = NULL;
        }
        
        // close stream
        if (stream != NULL ) {
            stream->Close();
            PvStream::Free(stream);
        }
        // disconnect device
        if (device != NULL ) {
            device->Disconnect();
            PvDevice::Free(device);
        }
    }

};

struct System {
    PvSystem* s;

    System() {
        s = new PvSystem();
    }

    ~System() {
        delete s;
    }
};

extern "C" {
#endif

    // make a state object
    EBUSState* mkstate() {
        std::list<PvBuffer*>* l;
        EBUSState* out = (struct EBUSState *) malloc(sizeof(EBUSState));
        out->device = NULL;
        out->stream = NULL;
        out->buffers = l;
        return out;
    }


    // return true if state has established connections
    bool w_is_active(EBUSState* state) {
        return (state->device != NULL) & (state->stream != NULL);
    }

    // find devices on the network
    const char* w_find(unsigned int timeout) {

        // define variables
        PvSystem pvsys;
        PvResult findResult;

        pvsys.SetDetectionTimeout(timeout);
        // find all devices on the network
        findResult = pvsys.Find();
        if ( findResult.IsOK() ) {
            // iterate over interfaces
            uint32_t nIface = pvsys.GetInterfaceCount();
            for ( uint32_t n = 0; n < nIface; n ++ ) {
                const PvInterface* iface = pvsys.GetInterface( n );
                const PvNetworkAdapter* NIC = dynamic_cast<const PvNetworkAdapter*>( iface );
                if ( NIC != NULL ) {
                    // scan host for attached devices
                    uint32_t mDev = iface->GetDeviceCount();
                    for ( uint32_t m = 0; m < mDev; m++ ) {
                        const PvDeviceInfo *devinfo = iface->GetDeviceInfo( m );
                        const PvDeviceInfoGEV* gigeinfo = dynamic_cast<const PvDeviceInfoGEV*>( devinfo );
                        if ( gigeinfo != NULL ) {
                            return devinfo->GetConnectionID().GetAscii();
                        }
                    }
                }
            }
        }
        // other than successful return
        return NULL;
    }

    // configure the device parameters over the GenAPI
    bool w_configure(EBUSState* state, const char* parameter, const char* value) {
        if (state == NULL ) { return false; }
        if (state->device == NULL ) { return false; }
        PvDeviceGEV* gige = dynamic_cast<PvDeviceGEV*>( state->device );
        PvGenParameterArray* array = gige->GetParameters();
        PvResult opr = array->SetEnumValue(PvString(parameter), PvString(value));
        return opr.IsOK();
    }

    // return the tick frequency
    long w_tick_frequency(EBUSState* state) {
        if (state == NULL ) { return false; }
        if (state->device == NULL ) { return false; }
        PvDeviceGEV* gige = dynamic_cast<PvDeviceGEV*>( state->device );
        PvGenParameterArray* array = gige->GetParameters();
        long value = 0;
        array->GetIntegerValue(PvString("GevTimestampTickFrequency"), value);
        return value;
    }



    // find, connect, and configure a connection
    void w_connect(EBUSState* state, const char* info, unsigned int timeout, int nbuffers) {

        // define variables
        PvResult opr;
        PvResult devres;
        PvDevice*  device;
        PvStream*  stream;
        std::string conn;

        // init buffer list storage
        std::list<PvBuffer*>* blist = state->buffers;
        blist = new std::list<PvBuffer*>();

        // ensure the connection info is not null
        if ( info != NULL ) { 
            // set the connection info
            conn = std::string(info);
            // connect to a device
            if ( conn.c_str() != NULL ) {
                device = PvDevice::CreateAndConnect( conn.c_str(), &devres );
                // if something went wrong the device is NULL
                if ( !devres.IsOK() ) { 
                    device = NULL; 
                }
            } else { device = NULL; }

            // if the device is not null
            if ( device != NULL ) {
                // open the stream
                if ( conn.c_str() != NULL ) {
                    stream = PvStream::CreateAndOpen( conn.c_str(), &opr );
                    // if the stream is not OK then set it to NULL
                    if ( !opr.IsOK() ) { 
                        stream = NULL; 
                    }
                } else { stream = NULL; }

                // now configure the stream
                if ( stream != NULL ) {
                    // configure the stream
                    if ( device != NULL && stream != NULL ) {
                        PvDeviceGEV* cam = dynamic_cast<PvDeviceGEV *>( device );
                        if ( cam != NULL ) {
                            PvStreamGEV *host = static_cast<PvStreamGEV *>( stream );
                            cam->NegotiatePacketSize();
                            cam->SetStreamDestination( host->GetLocalIPAddress(), host->GetLocalPort() );
                        }
                    }

                    // create stream buffers
                    if ( device != NULL && stream != NULL && nbuffers > 0 ) {

                        // Reading payload size from device
                        uint32_t sz = device->GetPayloadSize();

                        // Use BUFFER_COUNT or the maximum number of buffers, whichever is smaller
                        uint32_t total = ( stream->GetQueuedBufferMaximum() < nbuffers ) ? 
                            stream->GetQueuedBufferMaximum() :
                            nbuffers;

                        // Allocate buffers
                        for ( uint32_t i = 0; i < total; i++ )
                        {
                            // Create new buffer object
                            PvBuffer *buf = new PvBuffer();

                            // Have the new buffer object allocate payload memory
                            (*buf).Alloc( static_cast<uint32_t>( sz ) );
                            
                            // Add to external list - used to eventually release the buffers
                            (*blist).push_back( buf );

                            // queue the buffer in the stream
                            stream->QueueBuffer(buf);
                        }
                    }

                    // assign variables to state
                    state->buffers = blist;
                    state->device = device;
                    state->stream = stream;
                    // return now that the state is set
                    return;
                }
            }
        }

        /* set all to NULL */
        state->device = NULL;
        state->stream = NULL;
    }

    void w_shutdown(EBUSState *e) {
        // return if state is a null ptr
        if ( e != NULL ) {

            // stop acquisition
            if (e->device != NULL) {
                // get the command
                PvGenParameterArray *params = e->device->GetParameters();
                PvGenCommand *stop =  dynamic_cast<PvGenCommand *>( params->Get( "AcquisitionStop" ) );
                // stop streaming
                stop->Execute();
                e->device->StreamDisable();
            }

            // deal with stream buffers
            if (e->stream != NULL) {
                // abort the buffers
                e->stream->AbortQueuedBuffers();
                while ( e->stream->GetQueuedBufferCount() > 0 ) {
                    PvBuffer *buf = NULL;
                    PvResult opRes;
                    e->stream->RetrieveBuffer( &buf, &opRes );
                }
                if (e->buffers != NULL ) {

                    if (e->buffers->size() > 0) {
                        // free all buffers in the stream
                        std::list<PvBuffer*>::iterator it = (*(e->buffers)).begin();
                        while( it != (*(e->buffers)).end()) {
                            delete *it;
                            it++;
                        }
                        // Clear the buffer list 
                        (*(e->buffers)).clear();
                    }
                    // set list to NULL
                    e->buffers = NULL;
                }
            }
            
            // close and free the stream
            if ( e->stream != NULL ) {
                e->stream->Close();
                PvStream::Free( e->stream );
                e->stream = NULL;
            }

            // disconnect and free the device
            if ( e->device != NULL ) {
                e->device->Disconnect();
                PvDevice::Free( e->device );
                e->device = NULL;
            }
        }
    }

    // send command to start streaming
    void w_begin_streaming( EBUSState* state ) {
        if ( state == NULL ) { return; }
        if ( state->device == NULL ) { return; }
        if (state->device != NULL ) {
            // get the command
            PvGenParameterArray *params = state->device->GetParameters();
            PvGenCommand *start = dynamic_cast<PvGenCommand *>( params->Get( "AcquisitionStart" ) );
            // start streaming
            state->device->StreamEnable();
            start->Execute();
        }
    }


    // acquire a frame (timeout=1000) and return its tick value
    long w_acquire( EBUSState* state, unsigned char* buffer, int buflen, int timeout) {
        long ts = 0;

        // check inputs
        if ( buflen == 0 ) { return ts; }
        if ( buffer == NULL ) { return ts; }
        if ( state == NULL ) { return ts; }
        if ( state->device == NULL ) { return ts; }
        if ( state->stream == NULL ) { return ts; }


        // acquire a frame from the stream
        if ( state->stream != NULL ) {
            // define variables
            PvBuffer *sBuf = NULL;
            PvResult opr;
            // Retrieve next buffer
            PvResult res = state->stream->RetrieveBuffer( &sBuf, &opr, timeout );
            // process the buffer!
            if ( res.IsOK() && opr.IsOK() ) {
                // If the buffer contains an image, display width and height.
                if ( sBuf != NULL ) {
                    if ( sBuf->GetPayloadType() == PvPayloadTypeImage ) {
                        // determine number of elements to get
                        uint32_t num = ( sBuf->GetRequiredSize() < buflen ) ?
                            sBuf->GetRequiredSize():
                            buflen;
                        
                        // now copy data from the image to the buffer
                        std::memcpy(buffer, sBuf->GetImage()->GetDataPointer(), num);
                    }
                    // get the timestamp in tick's from the buffer
                    ts = sBuf->GetTimestamp();
                }
            }

            // requeue the buffer to the stream
            state->stream->QueueBuffer( sBuf );
        }
        return ts;
    }
#ifdef __cplusplus
}
#endif