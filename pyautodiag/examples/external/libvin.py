#pip install git+https://github.com/h3/python-libvin.git#egg=libvin
from libvin import Vin

def decode_vin(vin_code):
    v = Vin(vin_code)
    return {
        "region": v.region,
        "country": v.country,
        "year": v.year,
        "make": v.make,
        "manufacturer": v.manufacturer,
        "is_pre_2010": v.is_pre_2010,
        "wmi": v.wmi,
        "vds": v.vds,
        "vis": v.vis,
        "vsn": v.vsn,
        "less_than_500_built_per_year": v.less_than_500_built_per_year
    }

result = decode_vin("2A4GM684X6R632476")
print(result)