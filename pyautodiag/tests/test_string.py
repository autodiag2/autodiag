from autodiag.libloader import *
from autodiag.string import *

def test_string():
    s = object_string()
    list = list_object_string()
    list.append(s)
    assert list.size == 1
    list.append(s)
    assert list.size == 2
    list.remove_at(0)
    assert list.size == 1