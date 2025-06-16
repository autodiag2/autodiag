from autodiag.libloader import *

LogLevel = c_int
LOG_NONE = 0
LOG_ERROR = 1
LOG_WARNING = 2
LOG_INFO = 3
LOG_DEBUG = 4

lib.log_msg.argtypes = [LogLevel, c_char_p]
lib.log_msg.restype = None

lib.module_debug.argtypes = [c_char_p, c_char_p]
lib.module_debug.restype = None

lib.module_debug_init.argtypes = [c_char_p]
lib.module_debug_init.restype = None

lib.log_set_level.argtypes = [LogLevel]
lib.log_set_level.restype = None

lib.log_get_level.restype = LogLevel

lib.log_has_level.argtypes = [LogLevel]
lib.log_has_level.restype = bool

lib.log_level_from_str.argtypes = [c_char_p]
lib.log_level_from_str.restype = LogLevel

lib.log_level_to_str.argtypes = [LogLevel]
lib.log_level_to_str.restype = c_char_p

lib.log_get_current_time.restype = c_char_p

lib.log_set_from_env.restype = None

lib.log_is_env_set.restype = bool


def log_msg(msg: str, level=LOG_DEBUG, *args):
    formatted_msg = msg % args if args else msg
    lib.log_msg(level, formatted_msg.encode())

def module_debug(modname: str, msg: str):
    lib.module_debug(modname.encode(), msg.encode())

def module_debug_init(modname: str):
    lib.module_debug_init(modname.encode())

def log_set_level(level: int):
    lib.log_set_level(level)

def log_get_level() -> int:
    return lib.log_get_level()

def log_has_level(level: int) -> bool:
    return lib.log_has_level(level)

def log_level_from_str(level_str: str) -> int:
    return lib.log_level_from_str(level_str.encode())

def log_level_to_str(level: int) -> str:
    res = lib.log_level_to_str(level)
    return res.decode() if res else None

def log_get_current_time() -> str:
    res = lib.log_get_current_time()
    return res.decode() if res else None

def log_set_from_env():
    lib.log_set_from_env()

def log_is_env_set() -> bool:
    return lib.log_is_env_set()
