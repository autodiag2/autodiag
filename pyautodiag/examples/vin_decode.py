from autodiag.model.iso3779 import ISO3779
from autodiag.buffer import Buffer

decoder = ISO3779()
decoder.dump()
decoder.decode(Buffer.from_ascii("VF1BB05CF26010203"))
decoder.dump()
print(decoder.manufacturer_is_less_500())
print(decoder.region())