#!/opt/homebrew/bin/gawk -f

# For each SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE macro, extract the function name and PID string
/^SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE/ {
    args = ""
    # read lines until closing parenthesis
    while ( getline line ) {
        args = args line "\n"
        if ( line ~ /\)/ ) break
    }
    # extract the second and third comma‐separated arguments: name and pid
    if ( match(args, /\s*[^,]+\s*,\s*([^,]+)\s*,\s*([^,]+)\s*/, m) ) {
        name = m[1]
        pid  = m[2]
        print "object_hashmap_void_char_set(_saej1979_data_data_gen_pid_map, voiddup(" name "), strdup(" pid "));"
    }
}

