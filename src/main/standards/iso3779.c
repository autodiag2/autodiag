#include "standards/iso3779.h"

iso3779decoded * iso3779_vin_new() {
    iso3779decoded * vinDecoded = (iso3779decoded*)malloc(sizeof(iso3779decoded));
    vinDecoded->wmi.country = null;
    vinDecoded->wmi.manufacturer = null;
    vinDecoded->vis.serial_number = null;
    vinDecoded->vis.year = null;
    return vinDecoded;
}
void iso3779_vin_free(iso3779decoded *vin) {
    if ( vin->wmi.country != null ) {
        free(vin->wmi.country);
        vin->wmi.country = null;
    }
    if ( vin->wmi.manufacturer != null ) {
        free(vin->wmi.manufacturer);
        vin->wmi.manufacturer = null;
    }
    if ( vin->vis.serial_number != null ) {
        free(vin->vis.serial_number);
        vin->vis.serial_number = null;
    }
    if ( vin->vis.year != null ) {
        free(vin->vis.year);
        vin->vis.year = null;
    }
    free(vin);
}
char * iso3779decode_region_from(final Buffer *vin_raw) {
    final char * vin = buffer_to_ascii(vin_raw);
    if ( buffer_alphabet_compare(vin,"A","H") ) {
        return strdup("Africa");
    }
    if ( buffer_alphabet_compare(vin,"J","R") ) {
        return strdup("Asia");
    }
    if ( buffer_alphabet_compare(vin,"S","Z") ) {
        return strdup("Europe");
    }
    if ( buffer_alphabet_compare(vin,"1","5") ) {
        return strdup("North America");
    }
    if ( buffer_alphabet_compare(vin,"6","7") ) {
        return strdup("Oceania");
    }
    if ( buffer_alphabet_compare(vin,"8","9") ) {
        return strdup("South America");
    }
    return strdup("Unknown");
}

typedef struct {
    char start[3];
    char end[3];
    char country[100];
} ISO3779WmiCountry;

ISO3779WmiCountry ISO3779_wmi_countries[] = {
    {"AA", "AH", "South Africa"},
    {"AJ", "AN", "Ivory Coast"},
    {"AP", "A0", "Unassigned"},
    {"BA", "BE", "Angola"},
    {"BF", "BK", "Kenya"},
    {"BL", "BR", "Tanzania"},
    {"BS", "B0", "Unassigned"},
    {"CA", "CE", "Benin"},
    {"CF", "CK", "Madagascar"},
    {"CL", "CR", "Tunisia"},
    {"CS", "C0", "Unassigned"},
    {"DA", "DE", "Egypt"},
    {"DF", "DK", "Morocco"},
    {"DL", "DR", "Zambia"},
    {"DS", "D0", "Unassigned"},
    {"EA", "EE", "Ethiopia"},
    {"EF", "EK", "Mozambique"},
    {"EL", "E0", "Unassigned"},
    {"FA", "FE", "Ghana"},
    {"FF", "FK", "Nigeria"},
    {"FL", "F0", "Unassigned"},
    {"GA", "G0", "Unassigned"},
    {"HA", "H0", "Unassigned"},
    {"JA", "J0", "Japan"},
    {"KA", "KE", "Sri Lanka"},
    {"KF", "KK", "Israel"},
    {"KL", "KR", "South Korea"},
    {"KS", "K0", "Unassigned"},
    {"LA", "L0", "China"},
    {"MA", "ME", "India"},
    {"MF", "MK", "Indonesia"},
    {"ML", "MR", "Thailand"},
    {"MS", "M0", "Unassigned"},
    {"NF", "NK", "Pakistan"},
    {"NL", "NR", "Turkey"},
    {"NS", "N0", "Unassigned"},
    {"PA", "PE", "Philippines"},
    {"PF", "PK", "Singapore"},
    {"PL", "PR", "Malaysia"},
    {"PS", "P0", "Unassigned"},
    {"RA", "RE", "United Arab Emirates"},
    {"RF", "RK", "Taiwan"},
    {"RL", "RR", "Vietnam"},
    {"RS", "R0", "Unassigned"},
    {"SA", "SM", "Great Britain"},
    {"SN", "ST", "Germany"},
    {"SU", "SZ", "Poland"},
    {"S1", "S0", "Unassigned"},
    {"TA", "TH", "Switzerland"},
    {"TJ", "TP", "Czech Republic"},
    {"TR", "TV", "Hungary"},
    {"TW", "T1", "Portugal"},
    {"T2", "T0", "Unassigned"},
    {"UA", "UG", "Unassigned"},
    {"UH", "UM", "Denmark"},
    {"UN", "UT", "Ireland"},
    {"UU", "UZ", "Romania"},
    {"U1", "U4", "Unassigned"},
    {"U5", "U7", "Slovakia"},
    {"U8", "U0", "Unassigned"},
    {"VA", "VE", "Austria"},
    {"VF", "VR", "France"},
    {"VS", "VW", "Spain"},
    {"VX", "V2", "Yugoslavia"},
    {"V3", "V5", "Croatia"},
    {"V6", "V0", "Estonia"},
    {"WA", "W0", "Germany"},
    {"XA", "XE", "Bulgaria"},
    {"XF", "XK", "Greece"},
    {"XL", "XR", "Netherlands"},
    {"XS", "XW", "Russia"},
    {"XX", "X2", "Luxembourg"},
    {"X3", "X0", "Russia"},
    {"YA", "YE", "Belgium"},
    {"YF", "YK", "Finland"},
    {"YL", "YR", "Malta"},
    {"YS", "YW", "Sweden"},
    {"YX", "Y2", "Norway"},
    {"Y3", "Y5", "Belarus"},
    {"Y6", "Y0", "Ukraine"},
    {"ZA", "ZR", "Italy"},
    {"ZS", "ZW", "Unassigned"},
    {"ZX", "Z2", "Slovenia"},
    {"Z3", "Z5", "Lithuania"},
    {"Z6", "Z0", "Unassigned"},
    {"", "", ""} // Marqueur de fin
};

char * iso3779decode_country_from(final Buffer *vin_raw) {
    final char * vin = buffer_to_ascii(vin_raw);
    for (int i = 0; ISO3779_wmi_countries[i].start[0] != '\0'; i++) {
        if (buffer_alphabet_compare(vin, ISO3779_wmi_countries[i].start, ISO3779_wmi_countries[i].end)) {
            return strdup(ISO3779_wmi_countries[i].country);
        }
    }
    return strdup("Unassigned");
}
bool iso3779_wmi_manufacturer_is_less_500(final Buffer* vin) {
    return vin->buffer[2] == ISO3779_WMI_MANUFACTURER_LESS_500;
}

bool iso3779_wmi_manufacturers_read_tsv_line(BUFFER line, void*data) {
    void **ptrs = data;
    char * searched_wmi = (char*)ptrs[0];
    char **manufacturer = (char **)ptrs[1];

    if ( 0 < line->size ) {
        if ( line->buffer[0] == '#' ) {

        } else {
            char * firstTab = strchr(line->buffer,'\t');
            if ( firstTab != null ) {
                *firstTab = 0;
            }
            if ( strncasecmp(searched_wmi,line->buffer, strlen(line->buffer)) == 0 ) {
                if ( firstTab != null ) {
                    *manufacturer = strdup(firstTab + 1);
                }
            }
        }
    }

    return true;
}

bool iso3779_wmi_manufacturers_read_tsv(char *fileName, char * searched_wmi, char **manufacturer) {
    void ** ptrs = (void**)malloc(sizeof(void*)*2);
    ptrs[0] = searched_wmi;
    ptrs[1] = manufacturer;
    final bool res = file_read_lines(fileName,iso3779_wmi_manufacturers_read_tsv_line,ptrs);
    free(ptrs);
    return res;
}

char * iso3779decode_manufacturer_from(final Buffer *vin_raw) {
    final char * vin = buffer_to_ascii(vin_raw);
    char *manufacturers_file = config_get_in_data_folder_safe("data/VIN/manufacturers.tsv");
    if (manufacturers_file == NULL) {
        log_msg(LOG_ERROR, "Data directory not found, try reinstalling the software");
        return null;
    }
    char *manufacturer = null;
    if ( iso3779_wmi_manufacturer_is_less_500(vin_raw) ) {
        char manufacturer_code[4] = {0};
        memcpy(manufacturer_code,&vin_raw->buffer[11],3);
        log_msg(LOG_WARNING, "should get the manufacturer code: '%s' from vis", manufacturer_code);
    }
    if ( iso3779_wmi_manufacturers_read_tsv(manufacturers_file, vin, &manufacturer) ) {
        return manufacturer;
    } else {
        return strdup("Unknown manufacturer");
    }
}
char* ISO3779_vis_get_year_from(final Buffer *vin_raw) {
    switch (vin_raw->buffer[9]) {
        case 'M': return strdup("1991 or 2021");
        case 'N': return strdup("1992 or 2022");
        case 'P': return strdup("1993 or 2023");
        case 'R': return strdup("1994 or 2024");
        case 'S': return strdup("1995 or 2025");
        case 'T': return strdup("1996 or 2026");
        case 'V': return strdup("1997 or 2027");
        case 'W': return strdup("1998 or 2028");
        case 'X': return strdup("1999 or 2029");
        case 'Y': return strdup("2000 or 2030");
        case '1': return strdup("2001");
        case '2': return strdup("2002");
        case '3': return strdup("2003");
        case '4': return strdup("2004");
        case '5': return strdup("2005");
        case '6': return strdup("2006");
        case '7': return strdup("2007");
        case '8': return strdup("2008");
        case '9': return strdup("2009");
        case 'A': return strdup("2010");
        case 'B': return strdup("2011");
        case 'C': return strdup("2012");
        case 'D': return strdup("2013");
        case 'E': return strdup("2014");
        case 'F': return strdup("2015");
        case 'G': return strdup("2016");
        case 'H': return strdup("2017");
        case 'J': return strdup("2018");
        case 'K': return strdup("2019");
        case 'L': return strdup("2020");
        default: return strdup("Unknown year");
    }
}

char* ISO3779_vis_serial_number_from(final Buffer *vin_raw) {
    char * sn = null;
    if ( iso3779_wmi_manufacturer_is_less_500(vin_raw) ) {
        sn = strdup(vin_raw[14]);
    } else {
        sn = strdup(vin_raw[11]);
    }
    return sn;
}

/**
 * 1  2  3    4  5  6  7  8  9   10 11 12 13 14 15 16 17
 * WMI-----   VDS------          VIS-----------
 */
iso3779decoded* iso3779decode_from(final Buffer *vin) {
    iso3779decoded * vinDecoded = iso3779_vin_new();
    vinDecoded->wmi.country = iso3779decode_country_from(vin);
    vinDecoded->wmi.manufacturer = iso3779decode_manufacturer_from(vin);
    vinDecoded->vis.year = ISO3779_vis_get_year_from(vin);
    vinDecoded->vis.serial_number = ISO3779_vis_serial_number_from(vin);
    return vinDecoded;
}
