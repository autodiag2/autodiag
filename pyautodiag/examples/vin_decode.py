from autodiag.model.iso3779 import ISO3779
from autodiag.buffer import Buffer

decoder = ISO3779(Buffer.from_ascii("VF1BB05CF26010203"))
decoder.dump()
decoder.decode()
decoder.dump()
print(decoder.manufacturer_is_less_500())
print(decoder.region())