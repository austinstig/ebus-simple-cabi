use libc::{c_int, c_char};

#[link(name="wrapper", kind="dylib")]
extern {
    #[repr(C)]
    pub struct EBUSState { _unused: [u8;0] }
    pub fn mkstate() -> *mut EBUSState;
}