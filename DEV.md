
# Develop
## Prerequisite
#### Windows
 - Install [MSYS2](https://www.msys2.org/)
 - Open MSYS2 MINGW64 shell
 - Install Dependencies
 ```bash
 pacman -S mingw-w64-ucrt-x86_64-gtk3 git mingw-w64-ucrt-x86_64-toolchain base-devel gcc pkg-config mingw-w64-x86_64-gtk3 && \
  echo "export LIBRARY_PATH=/mingw64/lib:$LIBRARY_PATH" >> ~/.bashrc && \
  echo "export PKG_CONFIG_PATH=/mingw64/libautodiag/pkgconfig:/mingw64/share/pkgconfig" >> ~/.bashrc && \
  source ~/.bashrc
 ```
#### MacOS
```bash
brew install gtk+3
```
#### Debian based
```bash
sudo apt-get install make gcc build-essential debhelper dh-make devscripts libgtk-3-0 libgtk-3-dev git
```
## Build
```bash
git clone https://github.com/autodiag2/autodiag && \
 cd autodiag && \
 git submodule update --init --recursive && \
 make && \
 make install
```
## Run
```bash
./bin/autodiag
./bin/elm327sim
```

# Python package
## Build
For n architecture, on each run
```bash
make veryclean
make -j 8 installPython
```
Take the resulting lib (.so, .dylib, .dll) and paste it to pyautodiag/autodiag/libs/
Then
```bash
cd pyautodiag/
python setup.py sdist bdist_wheel
```
The package is ready