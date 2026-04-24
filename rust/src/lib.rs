pub mod sys;

use std::ffi::CString;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Method {
    Telex,
    Vni,
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum ToneStyle {
    Old, // hòa
    New, // hoà (Default)
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum FreeW {
    Off,
    NonStart,
    Always,
}

#[derive(Debug, Clone)]
pub struct EngineResult {
    pub action: u8,
    pub backspace: u8,
    pub count: u8,
    pub chars: Vec<char>,
}

pub struct LotusEngine {
    engine: *mut sys::lotus_engine_t,
}

impl LotusEngine {
    /// Create a new Lotus Engine instance.
    pub fn new() -> Self {
        let engine = unsafe { sys::lotus_engine_create() };
        assert!(!engine.is_null(), "Failed to create Lotus Engine instance");
        Self { engine }
    }

    /// Process a key press and return a transformation result.
    pub fn process_key(&mut self, key: u32, shift: bool, caps_lock: bool) -> EngineResult {
        let mods = sys::lotus_modifiers_t { shift, caps_lock };
        let res = unsafe { sys::lotus_engine_process_key(self.engine, key, mods) };
        
        let mut chars = Vec::with_capacity(res.count as usize);
        for i in 0..(res.count as usize) {
            if let Some(c) = char::from_u32(res.chars[i]) {
                chars.push(c);
            }
        }

        EngineResult {
            action: res.action,
            backspace: res.backspace,
            count: res.count,
            chars,
        }
    }

    /// Reset engine state.
    pub fn reset(&mut self) {
        unsafe { sys::lotus_engine_reset(self.engine) };
    }

    /// Configure the input method.
    pub fn set_method(&mut self, method: Method) {
        let c_method = match method {
            Method::Telex => sys::lotus_method_t::LOTUS_METHOD_TELEX,
            Method::Vni => sys::lotus_method_t::LOTUS_METHOD_VNI,
        };
        unsafe { sys::lotus_engine_set_method(self.engine, c_method) };
    }

    /// Configure the tone placement style.
    pub fn set_tone_style(&mut self, style: ToneStyle) {
        let c_style = match style {
            ToneStyle::Old => sys::lotus_tone_style_t::LOTUS_TONE_OLD,
            ToneStyle::New => sys::lotus_tone_style_t::LOTUS_TONE_NEW,
        };
        unsafe { sys::lotus_engine_set_tone_style(self.engine, c_style) };
    }

    /// Configure the Free-W option (Telex only).
    pub fn set_free_w(&mut self, option: FreeW) {
        let c_option = match option {
            FreeW::Off => sys::lotus_free_w_t::LOTUS_FREE_W_OFF,
            FreeW::NonStart => sys::lotus_free_w_t::LOTUS_FREE_W_NON_START,
            FreeW::Always => sys::lotus_free_w_t::LOTUS_FREE_W_ALWAYS,
        };
        unsafe { sys::lotus_engine_set_free_w(self.engine, c_option) };
    }

    /// Configure the manual hook keys option.
    pub fn set_std_uo(&mut self, enabled: bool) {
        unsafe { sys::lotus_engine_set_std_uo(self.engine, enabled) };
    }

    /// Add a custom shortcut for string expansion.
    pub fn add_shortcut(&mut self, trigger: &str, replacement: &str) {
        let c_trigger = CString::new(trigger).expect("CString::new failed for trigger");
        let c_replacement = CString::new(replacement).expect("CString::new failed for replacement");
        unsafe {
            sys::lotus_engine_add_shortcut(self.engine, c_trigger.as_ptr(), c_replacement.as_ptr());
        }
    }

    /// Enables or disables automatic English word restoration.
    pub fn set_auto_restore(&mut self, enabled: bool) {
        unsafe { sys::lotus_engine_set_auto_restore(self.engine, enabled) };
    }

    /// Enables or disables allowing non-standard initial consonants.
    pub fn set_allow_non_standard_initials(&mut self, enabled: bool) {
        unsafe { sys::lotus_engine_set_allow_non_standard_initials(self.engine, enabled) };
    }
}

impl Drop for LotusEngine {
    fn drop(&mut self) {
        unsafe { sys::lotus_engine_destroy(self.engine) };
    }
}
