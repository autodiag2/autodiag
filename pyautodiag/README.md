Manage a area or a group of people with more horizontal decision.
It assume a predation based world.
### Install
#### From source
```bash
conda create -n pyautodiag python=3.11 && \
    conda activate pyautodiag && \
    pip install git+https://github.com/autodiag2/pyautodiag.git#egg=pyautodiag
```
#### From GitHub
Download a release from [releases](https://github.com/autodiag2/pyautodiag/releases), then
```bash
conda create -n pyautodiag python=3.11 && \
    conda activate pyautodiag && \
    pip install pyautodiag-*.whl
```

### Develop
#### Install
```bash
conda create -y -n pyautodiag python=3.11 ; \
    conda activate pyautodiag && \
    pip install poetry && \
    poetry install --no-root && \
    pip install -e .
```
#### Build
```bash
poetry build
```
