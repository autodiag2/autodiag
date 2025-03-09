#include "standards/iso3779.h"

iso3779decoded * iso3779_vin_new() {
    iso3779decoded * vinDecoded = (iso3779decoded*)malloc(sizeof(iso3779decoded));
    vinDecoded->wmi.country = null;
    return vinDecoded;
}
void iso3779_vin_free(iso3779decoded *vin) {
    if ( vin->wmi.country != null ) {
        free(vin->wmi.country);
        vin->wmi.country = null;
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
    if ( iso3779_wmi_manufacturers_read_tsv(manufacturers_file, vin, &manufacturer) ) {
        return manufacturer;
    } else {
        return strdup("Unknown manufacturer");
    }
}
/**
 * 1  2  3    4  5  6  7  8  9   10 11 12 13 14 15 16 17
 * WMI-----   VDS------          VIS-----------
 */
iso3779decoded* iso3779decode_from(final Buffer *vin) {
    iso3779decoded * vinDecoded = iso3779_vin_new();
    vinDecoded->wmi.country = iso3779decode_country_from(vin);
    vinDecoded->wmi.manufacturer = iso3779decode_manufacturer_from(vin);
    return vinDecoded;
}
