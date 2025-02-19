#!/bin/bash

APP_NAME="autodiag"
APP_PATH="./bin/$APP_NAME.app"
APP_PACKAGE="./bin/$APP_NAME.dmg"
EXECS_PATH="$APP_PATH/Contents/MacOS/"
EXECUTABLE="$EXECS_PATH/$APP_NAME"
FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"

mkdir -p "$FRAMEWORKS_PATH"
mkdir -p "$EXECS_PATH"

for EXECUTABLE in bin/autodiag bin/elm327sim; do

    # Get the list of Homebrew dependencies
    DEPENDENCIES=$(otool -L "$EXECUTABLE" | grep -E "/opt/homebrew/" | awk '{print $1}')

    for LIB in $DEPENDENCIES; do
        LIB_NAME=$(basename "$LIB")

        # Copy library to the Frameworks folder if not already there
        if [ ! -f "$FRAMEWORKS_PATH/$LIB_NAME" ]; then
            cp "$LIB" "$FRAMEWORKS_PATH/"
        fi

        # Update the app binary to use the local Frameworks path
        install_name_tool -change "$LIB" "@executable_path/../Frameworks/$LIB_NAME" "$EXECUTABLE"
    done

    cp "$EXECUTABLE" "$EXECS_PATH"

done

# Ad-hoc sign the app
codesign --force --deep --sign - "$APP_PATH"
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_PATH" -ov -format UDZO "$APP_PACKAGE"

echo "Software ready at: $APP_PACKAGE"

