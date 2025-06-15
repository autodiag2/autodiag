#include "libautodiag/model/iso3779.h"

ISO3779_decoded * ISO3779_vin_new() {
    ISO3779_decoded * vinDecoded = (ISO3779_decoded*)malloc(sizeof(ISO3779_decoded));
    vinDecoded->wmi.country = null;
    vinDecoded->wmi.manufacturer = null;
    vinDecoded->vds.data = null;
    vinDecoded->vis.data = null;
    vinDecoded->vis.year = -1;
    return vinDecoded;
}
void ISO3779_vin_free(ISO3779_decoded *vin) {
    if ( vin->wmi.country != null ) {
        free(vin->wmi.country);
        vin->wmi.country = null;
    }
    if ( vin->wmi.manufacturer != null ) {
        free(vin->wmi.manufacturer);
        vin->wmi.manufacturer = null;
    }
    if ( vin->vds.data != null ) {
        free(vin->vds.data);
        vin->vds.data = null;
    }
    if ( vin->vis.data != null ) {
        free(vin->vis.data);
        vin->vis.data = null;
    }
    if ( vin->vis.year == - 1 ) {
        vin->vis.year = -1;
    }
    free(vin);
}
char * ISO3779_decode_region_from(final Buffer *vin_raw) {
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
} ISO3779_WmiCountry;

ISO3779_WmiCountry ISO3779_wmi_countries[] = {
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

char * ISO3779_decode_country_from(final Buffer *vin_raw) {
    final char * vin = buffer_to_ascii(vin_raw);
    for (int i = 0; ISO3779_wmi_countries[i].start[0] != '\0'; i++) {
        if (buffer_alphabet_compare(vin, ISO3779_wmi_countries[i].start, ISO3779_wmi_countries[i].end)) {
            return strdup(ISO3779_wmi_countries[i].country);
        }
    }
    return strdup("Unassigned");
}
bool ISO3779_wmi_manufacturer_is_less_500(final Buffer* vin) {
    return vin->buffer[2] == ISO3779_WMI_MANUFACTURER_LESS_500;
}

bool ISO3779_wmi_manufacturers_read_tsv_line(Buffer * line, void*data) {
    void **ptrs = data;
    char * searched_wmi = (char*)ptrs[0];
    char **manufacturer = (char **)ptrs[1];
    char *searched_manufacturer_code = (char *)ptrs[2];

    if ( 0 < line->size ) {
        if ( line->buffer[0] == '#' ) {

        } else {
            char * firstTab = strchr(line->buffer,'\t');
            char * secondTab = null;
            if ( firstTab != null ) {
                *firstTab = 0;
                secondTab = strchr(firstTab+1,'\t');
                if ( secondTab != null ) {
                    *secondTab = 0;
                }
            }
            if ( strncasecmp(searched_wmi,line->buffer, strlen(line->buffer)) == 0 ) {
                if ( firstTab != null ) {
                    if ( searched_manufacturer_code == null ) {
                        *manufacturer = strdup(firstTab + 1);
                    } else {
                        if ( secondTab != null ) {
                            if ( strncasecmp(searched_manufacturer_code,secondTab+1, strlen(secondTab+1)) == 0 ) {
                                *manufacturer = strdup(firstTab + 1);
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool ISO3779_wmi_manufacturers_read_tsv(char *fileName, char * searched_wmi, char **manufacturer, char *manufacturer_code) {
    void ** ptrs = (void**)malloc(sizeof(void*)*3);
    ptrs[0] = searched_wmi;
    ptrs[1] = manufacturer;
    ptrs[2] = manufacturer_code;
    final bool res = file_read_lines(fileName,ISO3779_wmi_manufacturers_read_tsv_line,ptrs);
    free(ptrs);
    return res;
}

char * ISO3779_decode_manufacturer_from(final Buffer *vin_raw) {
    final char * vin = buffer_to_ascii(vin_raw);
    char *manufacturers_file = installation_folder("data/vehicle/manufacturers.tsv");
    if (manufacturers_file == NULL) {
        log_msg(LOG_ERROR, "Data directory not found, try reinstalling the software");
        return null;
    }
    char *manufacturer = null;
    char *manufacturer_code = null;
    if ( ISO3779_wmi_manufacturer_is_less_500(vin_raw) ) {
        manufacturer_code = (char*)malloc(sizeof(char) * 4);
        strncpy(manufacturer_code,&vin_raw->buffer[11],3);
    }
    if ( ISO3779_wmi_manufacturers_read_tsv(manufacturers_file, vin, &manufacturer, manufacturer_code) ) {
        return manufacturer;
    } else {
        return strdup("Unknown manufacturer");
    }
}

const char YEAR_MAPPING[] = "123456789ABCDEFGHJKLMNPRSTVWXY";
#define YEAR_MAPPING_LENGTH (sizeof(YEAR_MAPPING) - 1)

int ISO3779_vis_get_year(char year_char, int current_year) {
    char *pos = strchr(YEAR_MAPPING, year_char);
    if (!pos) return -1;
    int period_offset = pos - YEAR_MAPPING;
    int offset_to_start = (current_year - 1971) % YEAR_MAPPING_LENGTH;
    int period_year = current_year - offset_to_start + period_offset;
    return period_year;
}

void ISO3779_dump(final Buffer *vin) {
    ISO3779_decoded * vinDecoded = ISO3779_decode_from(vin);
    char * region = ISO3779_decode_region_from(vin);
    printf("dump {\n");
    printf("    wmi {\n");
    printf("        region:         %s\n", region);
    printf("        country:        %s\n", vinDecoded->wmi.country);
    printf("        manufacturer:   %s\n", vinDecoded->wmi.manufacturer);
    printf("    }\n");
    printf("    vds: %s\n", vinDecoded->vds.data);
    printf("    vis: {\n");
    printf("        year: %d\n", vinDecoded->vis.year);
    printf("        data: %s\n", vinDecoded->vis.data);
    printf("    }\n");
    printf("}\n");
}

int get_current_year() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    return tm_info->tm_year + 1900;
}
/**
 * 1  2  3    4  5  6  7  8  9   10 11 12 13 14 15 16 17
 * WMI-----   VDS------          VIS-----------
 */
ISO3779_decoded* ISO3779_decode_from(final Buffer *vin) {
    ISO3779_decoded * vinDecoded = ISO3779_vin_new();
    vinDecoded->wmi.country = ISO3779_decode_country_from(vin);
    vinDecoded->wmi.manufacturer = ISO3779_decode_manufacturer_from(vin);
    vinDecoded->vds.data = bytes_to_hex_string(vin->buffer + 3, 6);
    vinDecoded->vis.year = ISO3779_vis_get_year(vin->buffer[3 + 6], get_current_year());
    vinDecoded->vis.data = bytes_to_hex_string(vin->buffer + 3 + 6 + 1, 8);
    return vinDecoded;
}
