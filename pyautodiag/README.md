### Install
#### From GitHub
Download a release from [releases](https://github.com/autodiag2/autodiag/releases), then
```bash
conda create -n autodiag python=3.11 && \
    conda activate autodiag && \
    pip install autodiag-*.whl
```

### Develop
#### Install
Install data folders and library
```bash
    make installDev installPythonDev
```
Create the conda environment and install autodiag module
```bash
conda create -y -n autodiag python=3.11 ; \
    conda activate autodiag && \
    pip install pytest && \
    pip install -e .
```
#### Build
For n architecture, on each run
```bash
make veryclean
make -j 8 compile_lib
```
Take the resulting lib (.so, .dylib, .dll) and paste it to ```pyautodiag/autodiag/libs/```  
Then to install data folders:
```bash
make installPython
```
Then make the python package:
```bash
cd pyautodiag/
python setup.py sdist bdist_wheel
```
The package is ready
#### Test
From ```pyautodiag/```
```bash
pytest
```