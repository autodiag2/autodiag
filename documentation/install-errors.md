#### MacOS
##### GSettings schemas
```
(autodiag:5985): GLib-GIO-ERROR **: 23:33:55.223: No GSettings schemas are installed on the system
```
Specify the correct location of gsettings schemas
```bash
echo "export GSETTINGS_SCHEMA_DIR=/opt/homebrew/share/glib-2.0/schemas/" >> ~/.bashrc && \
 source ~/.bashrc
```
#### Debian
##### Missing icons
```
(autodiag:43964): Gtk-WARNING **: 14:46:37.393: Error loading theme icon 'application-exit' for stock: Icon 'application-exit' not present in theme Yaru
````
Installed theme is missing some icons, install an other icon theme
```bash
sudo apt install gnome-icon-theme-symbolic
```
##### Library not found
```
./output/bin/autodiag: error while loading shared libraries: libautodiag-linux-x86_64.so: cannot open shared object file: No such file or directory
```
The shared library cannot be found: (adapt to the current location of the lib)
```
export LD_LIBRARY_PATH=$HOME/.local/lib/
```