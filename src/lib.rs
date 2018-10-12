extern crate num;

include!(concat!(env!("OUT_DIR"), "/wrapper.rs"));

use std::ffi::CStr;
use std::ffi::CString;
use num::ToPrimitive;
use std::mem::forget;
use std::os::raw::c_char;
use std::os::raw::c_uchar;

// link modules
mod device;

struct Error();
type ProgramResult<T> = Result<T, Error>;

// re-export symbols
pub use device::Device;


/// # Scan
/// The scan function will find all the devices on the network and
/// return the first GigE Vision device available. The returned Option
/// will contain the connection ID for the device.
pub fn scan<T: ToPrimitive+::std::fmt::Debug>(timeout: T) -> Option<String> {
    // perform the timeout conversion
    let t: u32 = timeout.to_u32().unwrap_or(0_u32);
    // get the const char* for the first valid device address
    // and convert the raw pointer to something rust likes
    let addr_ptr: *const c_char = unsafe { root::w_find(t) };
    // if the device address ptr is not null then cast it to a CStr
    // if the device address ptr is null then return None
    if !addr_ptr.is_null() {
        let opt = Some(unsafe { 
            CStr::from_ptr(addr_ptr)
                        .to_string_lossy()
                        .into_owned()
        });
        forget(addr_ptr);
        return opt;
    } else {
        None
    }
}

/// # Clock
/// The paramters of the device's clock ticks
#[derive(Default)]
struct Clock {
    base: u64,
    freq: u64,
}

/// # Connection
/// The connection information for a device
pub struct Connection(String);

impl Connection {
    /// construct a new connection string
    pub fn new(ptr: *const c_char) -> Option<Connection> {
        unsafe {
            CStr::from_ptr(ptr)
                .to_str()
                .ok()
                .map(String::from)
                .map(Connection)
        }
    }
}