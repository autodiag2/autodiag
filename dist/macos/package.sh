#!/bin/bash

APP_NAME="autodiag"
APP_PATH="./output/bin/$APP_NAME.app"
APP_PACKAGE="./output/bin/$APP_NAME.dmg"
EXECS_PATH="$APP_PATH/Contents/MacOS/"
EXECUTABLE="$EXECS_PATH/$APP_NAME"
FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"
RESOURCES_PATH="$APP_PATH/Contents/Resources"

. ./dist/macos/package_lib.sh

mkdir -p "$FRAMEWORKS_PATH"
mkdir -p "$EXECS_PATH"
mkdir -p "$RESOURCES_PATH"

for folder in ui data/data media ; do
    cp -fr "$folder" "$RESOURCES_PATH"
done

# Process all executables
for EXECUTABLE in output/bin/*; do
    [[ "$EXECUTABLE" == *.dylib ]] && continue
    [[ "$EXECUTABLE" == *.dmg ]] && continue
    [[ "$EXECUTABLE" == *.app ]] && continue
    process_dependencies "$EXECUTABLE" "$FRAMEWORKS_PATH"
    cp "$EXECUTABLE" "$EXECS_PATH"
done

# Process all libraries inside Frameworks folder recursively
for LIB in "$FRAMEWORKS_PATH"/*.dylib output/bin/*.dylib; do
    process_dependencies "$LIB" "$FRAMEWORKS_PATH"
done

for LIB in output/bin/*.dylib; do
    cp "$LIB" "$FRAMEWORKS_PATH"
    LIB_NAME=$(basename "$LIB")
    for EXEC in "$EXECS_PATH"/*; do
        install_name_tool -change "$LIB_NAME" "@executable_path/../Frameworks/$LIB_NAME" "$EXEC"
    done
done

# Ad-hoc sign the app
codesign --force --deep --sign - "$APP_PATH"
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_PATH" -ov -format UDZO "$APP_PACKAGE"

echo "Software ready at: $APP_PACKAGE"

