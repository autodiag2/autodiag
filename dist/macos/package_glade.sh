APP_NAME="glade"
APP_PATH="./bin/$APP_NAME.app"
APP_PACKAGE="./bin/$APP_NAME.dmg"
FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"

EXECS_PATH="$APP_PATH/Contents/MacOS/"
EXECUTABLE="$EXECS_PATH/glade"

. ./dist/macos/package_lib.sh

mkdir -p "$EXECS_PATH" "$FRAMEWORKS_PATH"

cp /opt/homebrew/bin/glade "$EXECS_PATH"
cp /opt/homebrew/Cellar/icu4c@76/76.1_1/libautodiag/libicudata.76.dylib "$FRAMEWORKS_PATH"
process_dependencies "$EXECUTABLE" "$FRAMEWORKS_PATH"

# Process all libraries inside Frameworks folder recursively
for LIB in "$FRAMEWORKS_PATH"/*.dylib; do
    process_dependencies "$LIB" "$FRAMEWORKS_PATH"
done

# Ad-hoc sign the app
codesign --force --deep --sign - "$APP_PATH"
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_PATH" -ov -format UDZO "$APP_PACKAGE"
codesign --force --deep --sign - "$APP_PACKAGE"

echo "Software ready at: $APP_PACKAGE"