
# Develop
## Prerequisite
#### Windows
 - Install [MSYS2](https://www.msys2.org/)
 - Open MSYS2 MINGW64 shell
 - Install Dependencies
 ```bash
 pacman -S mingw-w64-ucrt-x86_64-gtk3 git mingw-w64-ucrt-x86_64-toolchain base-devel gcc pkg-config mingw-w64-x86_64-gtk3 && \
  echo "export LIBRARY_PATH=/mingw64/lib:\$LIBRARY_PATH" >> ~/.bashrc && \
  echo "export PKG_CONFIG_PATH=/mingw64/lib/pkgconfig:/mingw64/share/pkgconfig" >> ~/.bashrc && \
  source ~/.bashrc
 ```
For the make target `distWindows`:
 - Install [InnoSetup](https://jrsoftware.org/isdl.php#stable)  

or:
```bash
choco install innosetup
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
### Retrieving the repo
```bash
git clone --depth=1 https://github.com/autodiag2/autodiag && \
 cd autodiag && \
 git submodule update --init --recursive --depth=1
```
### Compiling
```bash
make
```
### Cross compiling
```bash
export TOOLCHAIN=x86_64-w64-mingw32-
export TOOLCHAIN=i686-w64-mingw32-
export TOOLCHAIN=aarch64-linux-gnu-
make compile_lib
```
## Run
```bash
make installDev
./output/bin/autodiag
./output/bin/elm327sim
```
### Distribute
Making the package is possible only on the target system, ie distDebian can be done only from a debian-like distro, same for others.
```bash
make distDebian distWindows distMacOS
```

# Python package
See [here](/pyautodiag/README.md)

# Alternative simulator
```bash
python3 -m pip install ELM327-emulator
python3 -m elm
```