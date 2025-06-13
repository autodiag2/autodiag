from autodiag.libloader import *

lib.installation_folder.argtypes = [c_char_p]
lib.installation_folder.restype = c_char_p

def installation_folder(relative_path: str) -> str:
    res = lib.installation_folder(relative_path.encode())
    if res:
        return res.decode()
    return None
