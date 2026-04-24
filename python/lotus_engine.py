import ctypes
import os
import sys
from enum import IntEnum

# --- Constants & Enums ---
class Method(IntEnum):
    TELEX = 0
    VNI = 1

class ToneStyle(IntEnum):
    OLD = 0
    NEW = 1

class FreeW(IntEnum):
    OFF = 0
    NON_START = 1
    ALWAYS = 2

class LogLevel(IntEnum):
    DEBUG = 0
    INFO = 1
    ERROR = 2

# --- C Structs ---
class LotusResult(ctypes.Structure):
    _fields_ = [
        ("action", ctypes.c_uint8),
        ("backspace", ctypes.c_uint8),
        ("count", ctypes.c_uint8),
        ("chars", ctypes.c_uint32 * 32)
    ]

class LotusModifiers(ctypes.Structure):
    _fields_ = [
        ("shift", ctypes.c_bool),
        ("caps_lock", ctypes.c_bool)
    ]

# Callback type
LogCallbackType = ctypes.CFUNCTYPE(None, ctypes.c_int, ctypes.c_char_p)

# --- Library Loading ---
def _load_library():
    """Finds and loads the shared library."""
    lib_name = "liblotus_engine_core.so"
    # Depending on the OS, we might want to change the extension (e.g., .dylib on macOS, .dll on Windows)
    if sys.platform == "darwin":
        lib_name = "liblotus_engine_core.dylib"
    elif sys.platform == "win32":
        lib_name = "lotus_engine_core.dll"

    # Default to current directory or standard paths
    search_paths = [
        os.path.abspath(os.path.dirname(__file__)),
        os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build")),
        os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build", "src", "api")),
    ]

    for path in search_paths:
        full_path = os.path.join(path, lib_name)
        if os.path.exists(full_path):
            try:
                return ctypes.CDLL(full_path)
            except OSError as e:
                print(f"Failed to load {full_path}: {e}")
    
    # Try system paths if not found locally
    try:
        return ctypes.CDLL(lib_name)
    except OSError:
        raise FileNotFoundError(f"Could not find the shared library: {lib_name}. Ensure it is compiled and in the path.")

_lib = _load_library()

# --- C API Setup ---
_lib.lotus_engine_create.restype = ctypes.c_void_p
_lib.lotus_engine_create.argtypes = []

_lib.lotus_engine_destroy.restype = None
_lib.lotus_engine_destroy.argtypes = [ctypes.c_void_p]

_lib.lotus_engine_process_key.restype = LotusResult
_lib.lotus_engine_process_key.argtypes = [ctypes.c_void_p, ctypes.c_uint32, LotusModifiers]

_lib.lotus_engine_reset.restype = None
_lib.lotus_engine_reset.argtypes = [ctypes.c_void_p]

_lib.lotus_engine_set_method.restype = None
_lib.lotus_engine_set_method.argtypes = [ctypes.c_void_p, ctypes.c_int]

_lib.lotus_engine_set_tone_style.restype = None
_lib.lotus_engine_set_tone_style.argtypes = [ctypes.c_void_p, ctypes.c_int]

_lib.lotus_engine_set_free_w.restype = None
_lib.lotus_engine_set_free_w.argtypes = [ctypes.c_void_p, ctypes.c_int]

_lib.lotus_engine_set_std_uo.restype = None
_lib.lotus_engine_set_std_uo.argtypes = [ctypes.c_void_p, ctypes.c_bool]

_lib.lotus_engine_add_shortcut.restype = None
_lib.lotus_engine_add_shortcut.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]

_lib.lotus_engine_set_log_callback.restype = None
_lib.lotus_engine_set_log_callback.argtypes = [LogCallbackType]

_lib.lotus_engine_set_auto_restore.restype = None
_lib.lotus_engine_set_auto_restore.argtypes = [ctypes.c_void_p, ctypes.c_bool]


# --- Python Wrapper ---
class LotusEngine:
    """Python wrapper for the Lotus Engine C-API."""

    def __init__(self):
        """Initializes a new engine instance."""
        self._engine = _lib.lotus_engine_create()
        if not self._engine:
            raise RuntimeError("Failed to create lotus_engine_t instance.")
        # Keep a reference to callbacks to prevent garbage collection
        self._log_callback_ref = None

    def __del__(self):
        """Destroys the engine instance."""
        if hasattr(self, '_engine') and self._engine:
            _lib.lotus_engine_destroy(self._engine)

    def process_key(self, key_char: str, shift: bool = False, caps_lock: bool = False) -> tuple:
        """
        Processes a single key press.

        Args:
            key_char: The character pressed (e.g., 'a', 'W').
            shift: True if Shift is held.
            caps_lock: True if Caps Lock is active.

        Returns:
            A tuple containing (action, backspace, output_string).
        """
        if not key_char:
            return (0, 0, "")
            
        # Get unicode codepoint
        keycode = ord(key_char[0])
        
        mods = LotusModifiers(shift=shift, caps_lock=caps_lock)
        
        res = _lib.lotus_engine_process_key(self._engine, keycode, mods)
        
        output_str = ""
        if res.count > 0:
            # Reconstruct string from UTF-32 array
            output_chars = [chr(res.chars[i]) for i in range(res.count)]
            output_str = "".join(output_chars)
            
        return (res.action, res.backspace, output_str)

    def reset(self):
        """Resets the engine state."""
        _lib.lotus_engine_reset(self._engine)

    def set_method(self, method: Method):
        """Sets the input method (TELEX or VNI)."""
        _lib.lotus_engine_set_method(self._engine, int(method))

    def set_tone_style(self, style: ToneStyle):
        """Sets the tone placement style (OLD or NEW)."""
        _lib.lotus_engine_set_tone_style(self._engine, int(style))

    def set_free_w(self, option: FreeW):
        """Sets the Free-W option."""
        _lib.lotus_engine_set_free_w(self._engine, int(option))

    def set_std_uo(self, enabled: bool):
        """Enables or disables manual hook keys ([ and ] -> ư, ơ)."""
        _lib.lotus_engine_set_std_uo(self._engine, enabled)

    def add_shortcut(self, trigger: str, replacement: str):
        """Adds a custom shortcut."""
        _lib.lotus_engine_add_shortcut(self._engine, trigger.encode('utf-8'), replacement.encode('utf-8'))
        
    def set_auto_restore(self, enabled: bool):
        """Enables or disables automatic English word restoration."""
        _lib.lotus_engine_set_auto_restore(self._engine, enabled)

    @staticmethod
    def set_log_callback(callback):
        """
        Sets a global logging callback.
        
        Args:
            callback: A Python function taking (level: int, message: str)
        """
        def _wrapper(level, message):
            callback(level, message.decode('utf-8'))
            
        cb = LogCallbackType(_wrapper)
        # Store callback globally to prevent GC issues since it's a global C API function
        LotusEngine._global_log_callback = cb 
        _lib.lotus_engine_set_log_callback(cb)
