Manage a area or a group of people with more horizontal decision.
It assume a predation based world.
### Install
#### From source
```bash
conda create -n autodiag python=3.11 && \
    conda activate autodiag && \
    pip install git+https://github.com/autodiag2/autodiag.git#egg=autodiag
```
#### From GitHub
Download a release from [releases](https://github.com/autodiag2/autodiag/releases), then
```bash
conda create -n autodiag python=3.11 && \
    conda activate autodiag && \
    pip install autodiag-*.whl
```

### Develop
#### Install
```bash
conda create -y -n autodiag python=3.11 ; \
    conda activate autodiag && \
    pip install poetry && \
    poetry install --no-root && \
    pip install -e .
```
#### Build
```bash
poetry build
```
