from autodiag.buffer import Buffer
from autodiag.log import *

def test_simple():
    b = Buffer()
    b.append_byte(0x01)
    assert b.buffer[0] == 0x01

    b = Buffer()
    b.append_byte(0x01)
    b.append_bytes([0x02, 0x03, 0x04])
    assert b.equals(Buffer.from_ascii_hex("01020304"))
    b.append_str("Hello")
    assert b.equals(Buffer.from_ascii("\x01\x02\x03\x04Hello"))
    b2 = Buffer.from_ascii_hex("48656c6c6f")
    b.prepend(b2)
    assert b.equals(Buffer.from_ascii("\x48\x65\x6c\x6c\x6f\x01\x02\x03\x04Hello"))
    b.ensure_termination()
    assert b.get_free_space() == 0
    b3 = b.copy()
    assert b.equals(b3)
    b4 = Buffer.new_random(10)
    assert b4.size == 10
    b5 = Buffer.new_cycle(10, 50)
    assert b5.size == 10
    s = "Test ASCII"
    b7 = Buffer.from_ascii(s)
    assert b7.size == len(s)
    b8 = Buffer.from_ascii_hex("54657374")
    assert b8.size == 4
    b.padding(20, 0xFF)
    assert 20 == b.size