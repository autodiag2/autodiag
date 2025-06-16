from autodiag.model.iso3779 import ISO3779

decoder = ISO3779()
decoder.dump()
decoder.decode("VF1BB05CF26010203")
decoder.dump()
print(decoder.manufacturer_is_less_500())
print(decoder.region())
print(decoder.get_country())
print(decoder.get_manufacturer())
print(decoder.year)