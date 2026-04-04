from autodiag.model.iso3779 import ISO3779
from autodiag.log import *

def test_decode():
    decoder = ISO3779()
    decoder.decode("VF1BB05CF26010203")
    assert decoder.region().lower() == "europe"
    assert decoder.get_country().lower() == "france"
    assert "renault" in decoder.get_manufacturer().lower()
    assert decoder.year == 2002