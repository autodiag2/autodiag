#### MacOS
```
(autodiag:5985): GLib-GIO-ERROR **: 23:33:55.223: No GSettings schemas are installed on the system
```
Specify the correct location of gsettings schemas
```bash
echo "export GSETTINGS_SCHEMA_DIR=/opt/homebrew/share/glib-2.0/schemas/" >> ~/.bashrc && \
 source ~/.bashrc
```
#### Debian
```
(autodiag:43964): Gtk-WARNING **: 14:46:37.393: Error loading theme icon 'application-exit' for stock: Icon 'application-exit' not present in theme Yaru
````
Installed theme is missing some icons, install an other icon theme
```bash
sudo apt install gnome-icon-theme-symbolic
```