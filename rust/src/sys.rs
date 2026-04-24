#![allow(non_camel_case_types)]

use libc::{c_char, c_double};

#[repr(C)]
pub struct lotus_engine_t {
    _private: [u8; 0],
}

#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum lotus_method_t {
    LOTUS_METHOD_TELEX = 0,
    LOTUS_METHOD_VNI = 1,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum lotus_tone_style_t {
    LOTUS_TONE_OLD = 0,
    LOTUS_TONE_NEW = 1,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum lotus_free_w_t {
    LOTUS_FREE_W_OFF = 0,
    LOTUS_FREE_W_NON_START = 1,
    LOTUS_FREE_W_ALWAYS = 2,
}

#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum lotus_log_level_t {
    LOTUS_LOG_LEVEL_DEBUG = 0,
    LOTUS_LOG_LEVEL_INFO = 1,
    LOTUS_LOG_LEVEL_WARN = 2,
    LOTUS_LOG_LEVEL_ERROR = 3,
}

pub type lotus_log_callback_t = Option<
    unsafe extern "C" fn(
        level: lotus_log_level_t,
        stage: *const c_char,
        time_us: c_double,
        message: *const c_char,
    ),
>;

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct lotus_result_t {
    pub action: u8,
    pub backspace: u8,
    pub count: u8,
    pub chars: [u32; 32],
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct lotus_modifiers_t {
    pub shift: bool,
    pub caps_lock: bool,
}

extern "C" {
    pub fn lotus_engine_create() -> *mut lotus_engine_t;
    pub fn lotus_engine_destroy(engine: *mut lotus_engine_t);
    pub fn lotus_engine_process_key(
        engine: *mut lotus_engine_t,
        key: u32,
        mods: lotus_modifiers_t,
    ) -> lotus_result_t;
    pub fn lotus_engine_reset(engine: *mut lotus_engine_t);
    pub fn lotus_engine_set_method(engine: *mut lotus_engine_t, method: lotus_method_t);
    pub fn lotus_engine_set_tone_style(engine: *mut lotus_engine_t, style: lotus_tone_style_t);
    pub fn lotus_engine_set_free_w(engine: *mut lotus_engine_t, option: lotus_free_w_t);
    pub fn lotus_engine_set_std_uo(engine: *mut lotus_engine_t, enabled: bool);
    pub fn lotus_engine_add_shortcut(
        engine: *mut lotus_engine_t,
        trigger: *const c_char,
        replacement: *const c_char,
    );
    pub fn lotus_engine_set_log_callback(callback: lotus_log_callback_t);
    pub fn lotus_engine_set_auto_restore(engine: *mut lotus_engine_t, enabled: bool);
    pub fn lotus_engine_set_allow_non_standard_initials(engine: *mut lotus_engine_t, enabled: bool);
    pub fn lotus_engine_export_tracing(filepath: *const c_char);
}
