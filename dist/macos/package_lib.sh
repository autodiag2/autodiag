function process_dependencies() {
    local TARGET=$1  # Executable or library file to process
    local FRAMEWORKS_PATH=$2
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