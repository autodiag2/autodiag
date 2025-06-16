from autodiag.model.iso3779 import ISO3779
from autodiag.log import *

def test_decode():
    decoder = ISO3779()
    decoder.decode("VF1BB05CF26010203")
    assert decoder.manufacturer_is_less_500() == False
    assert decoder.region() == "Europe"
    assert decoder.get_country() == "france"
    assert "enault" in decoder.get_manufacturer()
    assert decoder.year == 2002