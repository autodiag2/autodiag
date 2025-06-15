from autodiag.buffer import Buffer

def test_simple():
    b = Buffer()
    b.append_byte(0x01)
    assert b.buffer[0] == 0x01