use super::*;

/// # Device
/// A wrapper for the device.
pub struct Device {
    // connection information
    connection: Connection,
    // storage for the tick freqencies
    clock: Clock,
    // the ebus state
    state: Option<*mut root::EBUSState>
}

impl Device {


    /// construct a new Device based on the connection id
    pub fn new<S: AsRef<str>, T: ToPrimitive>(conn: S, timeout: T, nbuffers: T) -> Option<Device> {
        // handle input args
        let nbufs = nbuffers.to_i32().unwrap_or(0_i32);
        let t = timeout.to_u32().unwrap_or(0_u32);
        // construct a new state
        let state: *mut root::EBUSState = unsafe { root::mkstate() };
        // connect to the device via connection ID
        //debug!("CONNECT {:?}", conn.as_ref());
        let address: Option<*const c_char> = if let Ok(cstring) = CString::new(conn.as_ref()) {
            // get a ptr to the cstring
            let cptr = cstring.as_ptr();
            // clean up the memory for the cstring
            forget(cstring);
            // return the const char*
            Some(cptr)
        } else { None };
        // perform the connection to the device
        if let Some(addr) = address {
            Device::connect(addr, state, t, nbufs)
        } else{ None }
    }

    /// connect to a device
    fn connect(connection_id: *const c_char, 
               state: *mut root::EBUSState, 
               timeout: u32, 
               nbuffers: i32) -> Option<Device> {
        // perform the connection to the device
        unsafe { root::w_connect(state, connection_id, timeout, nbuffers) };
        // set the connection obj
        let connection = Connection::new(connection_id)?;
        // set the clock obj
        // TODO: lookup and form the clock object!
        let clock = Clock::default();
        // construct the device object
        Some(Device {
            connection: connection,
            clock: clock,
            state: Some(state)
        })
    }

    /// configure the device parameters
    pub fn configure<S: AsRef<str>>(&self, param: S, value: S) -> bool {
        // convert parameter input
        let p: Option<*const c_char> = if let Ok(cstring) = CString::new(param.as_ref()) {
            // get a ptr to the cstring
            let cptr = cstring.as_ptr();
            // clean up the memory for the cstring
            forget(cstring);
            // return the const char*
            Some(cptr)
        } else { None };

        // convert value input
        let v: Option<*const c_char> = if let Ok(cstring) = CString::new(value.as_ref()) {
            // get a ptr to the cstring
            let cptr = cstring.as_ptr();
            // clean up the memory for the cstring
            forget(cstring);
            // return the const char*
            Some(cptr)
        } else { None };

        // update the device parameters
        if let (Some(state), Some(pp), Some(vv)) = (self.state, p,v) {
            unsafe { root::w_configure(state, pp, vv) }
        } else { false }
    }

    /// get the tick frequency of the device
    pub fn tick_frequency(&self) -> u64 {
        if let Some(state) = self.state {
            unsafe { root::w_tick_frequency(state) as u64 }
        } else { 0u64 }
    }


    /// return true if the Device is fully active
    pub fn is_active(&self) -> bool {
        if let Some(state) = self.state {
            unsafe { root::w_is_active(state) }
        } else { false }
    }

    /// begin streaming frames from the device
    pub fn stream(&mut self) {
        // get a mut ref to the state ptr
        if let Some(mut state) = self.state {
            // call the begin streaming method for the state
            unsafe { root::w_begin_streaming(state); }
            //debug!("BEGIN STREAMING");
        }
    }


    /// acquire data from the device by copying a buffer
    pub fn acquire(&self, buf: &mut [u8], timeout: i32) -> usize {
        let mut ts: i64 = 0;
        if let Some(mut state) = self.state {
            let mut abuffer = buf.as_mut_ptr() as *mut std::os::raw::c_uchar;
            let size: i32 = buf.len() as i32;
            unsafe { 
                ts = root::w_acquire(state, abuffer, size, timeout);
            };
        }
        return ts as usize;
    }

    /// shutdown the device freeing the resources
    fn shutdown(&mut self) {
        println!("shutdown!");
        // get a mut ref to the state ptr
        if let Some(mut state) = self.state {
            //debug!("SHUTDOWN DEVICE");
            // call the shutdown method for the state
            unsafe { root::w_shutdown(state); }
        }
        self.state = None
    }
}

impl Drop for Device {
    fn drop(&mut self) {
        self.shutdown();
    }
}