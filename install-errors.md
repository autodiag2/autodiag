#### MacOS
```
(autodiag:5985): GLib-GIO-ERROR **: 23:33:55.223: No GSettings schemas are installed on the system
```
Specify the correct location of gsettings schemas
```bash
echo "export GSETTINGS_SCHEMA_DIR=/opt/homebrew/share/glib-2.0/schemas/" >> ~/.bashrc && \
 source ~/.bashrc
```