APP_NAME="glade"
APP_PATH="./bin/$APP_NAME.app"
APP_PACKAGE="./bin/$APP_NAME.dmg"

EXECS_PATH="$APP_PATH/Contents/MacOS/"

mkdir -p "$EXECS_PATH"
cp /opt/homebrew/bin/glade "$EXECS_PATH"

# Ad-hoc sign the app
codesign --force --deep --sign - "$APP_PATH"
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_PATH" -ov -format UDZO "$APP_PACKAGE"

echo "Software ready at: $APP_PACKAGE"