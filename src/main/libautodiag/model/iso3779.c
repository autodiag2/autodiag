#include "libautodiag/model/iso3779.h"

char * ISO3779_region(final ISO3779 *decoder) {
    final char * vin = buffer_to_ascii(decoder->vin);
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

ISO3779 * ISO3779_new() {
    final ISO3779 * decoder = (ISO3779*)malloc(sizeof(ISO3779));
    decoder->vin = null;
    decoder->country = null;
    decoder->manufacturer = null;
    decoder->year = -1;
    decoder->wmi = null;
    decoder->vds = null;
    decoder->vis = null;
    return decoder;
}
void ISO3779_dump(final ISO3779 *decoder) {
    printf("ISO3779: {\n");
    printf("    country: %s\n", decoder->country);
    printf("    manufacturer: %s\n", decoder->manufacturer);
    printf("    year: %d\n", decoder->year);
    printf("    wmi: %s\n", decoder->wmi);
    printf("    vds: %s\n", decoder->vds);
    printf("    vis: %s\n", decoder->vis);
    printf("    vin: %s\n", buffer_to_ascii(decoder->vin));
    printf("}\n");
}
void ISO3779_free(ISO3779 *decoder) {
    buffer_free(decoder->vin);
    decoder->vin = null;
    MEMORY_FREE_POINTER(decoder->country)
    MEMORY_FREE_POINTER(decoder->manufacturer)
    decoder->year = -1;
    decoder->wmi = null;
    decoder->vds = null;
    decoder->vis = null;
    free(decoder);
}

bool ISO3779_country_read_tsv_line(Buffer * line, void*data) {
    void** ptrs = (void**)data; 
    char *vin_prefix = (char *)ptrs[0];
    char **result = (char**)ptrs[1];
    char *start = strtok(line->buffer, "\t");
    char *end = strtok(NULL, "\t");
    char *country = strtok(NULL, "\t");
    if (!start || !end || !country) return true;
    if (buffer_alphabet_compare(vin_prefix, start, end)) {
        *result = strdup(country);
        return false;
    }
    return true;
}
char * ISO3779_country(final ISO3779 *decoder) {
    final char *wmi = decoder->wmi;
    char *result = NULL;

    char *countries_file = installation_folder("data/vehicle/countries.tsv");
    if (countries_file == NULL) {
        log_msg(LOG_ERROR, "Data directory not found, try reinstalling the software");
        return null;
    }

    char vin_prefix[] = { wmi[0], wmi[1], '\0' };
    void* parameters[] = {vin_prefix, &result};
    file_read_lines(countries_file, ISO3779_country_read_tsv_line, parameters);
    return result ? result : strdup("Unassigned");
}

bool ISO3779_manufacturers_read_tsv_line(Buffer * line, void*data) {
    void **ptrs = data;
    char * searched_wmi = (char*)ptrs[0];
    char **manufacturer = (char **)ptrs[1];
    char *searched_manufacturer_code = (char *)ptrs[2];

    if ( 0 < line->size ) {
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

    return true;
}

char * ISO3779_manufacturer(final ISO3779 *decoder) {
    final char * vin = buffer_to_ascii(decoder->vin);
    char *manufacturers_file = installation_folder("data/vehicle/manufacturers.tsv");
    if (manufacturers_file == NULL) {
        log_msg(LOG_ERROR, "Data directory not found, try reinstalling the software");
        return null;
    }
    char *manufacturer = null;
    char *manufacturer_code = null;
    if ( ISO3779_manufacturer_is_less_500(decoder) ) {
        manufacturer_code = (char*)malloc(sizeof(char) * 4);
        strncpy(manufacturer_code,&decoder->vis[2],3);
    }
    void* parameters[] = {vin, &manufacturer, manufacturer_code};
    if ( file_read_lines(manufacturers_file,ISO3779_manufacturers_read_tsv_line,parameters) ) {
        return manufacturer;
    } else {
        return strdup("Unknown manufacturer");
    }
}

const char YEAR_MAPPING[] = "123456789ABCDEFGHJKLMNPRSTVWXY";
#define YEAR_MAPPING_LENGTH (sizeof(YEAR_MAPPING) - 1)

int ISO3779_year(final ISO3779 *decoder, final int current_year) {
    final int year_char = decoder->vis[0];
    char *pos = strchr(YEAR_MAPPING, year_char);
    if (!pos) return -1;
    int period_offset = pos - YEAR_MAPPING;
    int offset_to_start = (current_year - 1971) % YEAR_MAPPING_LENGTH;
    final int period_year = current_year - offset_to_start + period_offset;
    return period_year;
}
int ISO3779_year_recent(final ISO3779 *decoder) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    final int current_year = tm_info->tm_year + 1900;
    return ISO3779_year(decoder, current_year);
}

void ISO3779_decode_internal(final ISO3779 *decoder, final Buffer *vin) {
    assert(17 <= vin->size);
    decoder->vin = buffer_copy(vin);
    decoder->country = null;
    decoder->manufacturer = null;
    decoder->year = -1;
    decoder->wmi = decoder->vin->buffer;
    decoder->vds = decoder->vin->buffer + 3;
    decoder->vis = decoder->vin->buffer + 3 + 6;
    decoder->country = ISO3779_country(decoder);
    decoder->manufacturer = ISO3779_manufacturer(decoder);
}

void ISO3779_decode_at_year(final ISO3779 *decoder, final Buffer *vin, final int year) {
    ISO3779_decode_internal(decoder, vin);
    decoder->year = ISO3779_year(decoder, year);
}

void ISO3779_decode(final ISO3779 *decoder, final Buffer *vin) {
    ISO3779_decode_internal(decoder, vin);
    decoder->year = ISO3779_year_recent(decoder);
}

bool ISO3779_manufacturer_is_less_500(final ISO3779 *decoder) {
    return decoder->wmi[2] == ISO3779_WMI_MANUFACTURER_LESS_500;
}
