#!/bin/bash

APP_NAME="autodiag"
APP_PATH="./bin/$APP_NAME.app"
APP_PACKAGE="./bin/$APP_NAME.dmg"
EXECS_PATH="$APP_PATH/Contents/MacOS/"
EXECUTABLE="$EXECS_PATH/$APP_NAME"
FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"
RESOURCES_PATH="$APP_PATH/Contents/Resources"

mkdir -p "$FRAMEWORKS_PATH"
mkdir -p "$EXECS_PATH"
mkdir -p "$RESOURCES_PATH"

for folder in ui data/data media ; do
    cp -fr "$folder" "$RESOURCES_PATH"
done

function process_dependencies() {
    local TARGET=$1  # Executable or library file to process
    local PROCESSED_LIBS=()

    while true; do
        NEW_LIBS_FOUND=false

        # Get the list of Homebrew dependencies
        DEPENDENCIES=$(otool -L "$TARGET" | grep -E "/opt/homebrew/|/usr/local/" | awk '{print $1}')

        for LIB in $DEPENDENCIES; do
            LIB_NAME=$(basename "$LIB")

            # Skip already processed libraries
            if [[ " ${PROCESSED_LIBS[@]} " =~ " ${LIB_NAME} " ]]; then
                continue
            fi

            # Copy library if not already present
            if [ ! -f "$FRAMEWORKS_PATH/$LIB_NAME" ]; then
                cp "$LIB" "$FRAMEWORKS_PATH/"
                NEW_LIBS_FOUND=true
            fi

            # Update the binary to reference the local Frameworks path
            install_name_tool -change "$LIB" "@executable_path/../Frameworks/$LIB_NAME" "$TARGET"

            # Mark library as processed
            PROCESSED_LIBS+=("$LIB_NAME")
        done

        # If no new libraries were found, exit the loop
        [ "$NEW_LIBS_FOUND" = false ] && break
    done
}

# Process all executables
for EXECUTABLE in bin/autodiag bin/elm327sim; do
    process_dependencies "$EXECUTABLE"
    cp "$EXECUTABLE" "$EXECS_PATH"
done

# Process all libraries inside Frameworks folder recursively
for LIB in "$FRAMEWORKS_PATH"/*.dylib; do
    process_dependencies "$LIB"
done

# Ad-hoc sign the app
codesign --force --deep --sign - "$APP_PATH"
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_PATH" -ov -format UDZO "$APP_PACKAGE"

echo "Software ready at: $APP_PACKAGE"

