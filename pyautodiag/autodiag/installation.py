from autodiag.libloader import *

lib.installation_folder_resolve.argtypes = [c_char_p]
lib.installation_folder_resolve.restype = c_char_p

def installation_folder_resolve(relative_path: str) -> str:
    res = lib.installation_folder_resolve(relative_path.encode())
    if res:
        return res.decode()
    return None
