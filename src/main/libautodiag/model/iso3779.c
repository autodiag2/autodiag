#include "libautodiag/model/iso3779.h"
#include "sqlite3.h"

static sqlite3 *ISO3779_db_open() {
    char *sqlite_path = installation_folder_resolve("data/ad_database.sqlite");
    if (sqlite_path == null) {
        log_msg(LOG_ERROR, "Cannot resolve sqlite database path");
        return null;
    }

    sqlite3 *db = null;
    int rc = sqlite3_open(sqlite_path, &db);
    free(sqlite_path);

    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_open failed: %s", db != null ? sqlite3_errmsg(db) : "unknown error");
        if (db != null) {
            sqlite3_close(db);
        }
        return null;
    }

    return db;
}

static char *ISO3779_db_select_text(sqlite3 *db, const char *sql, const char *wmi3, const char *wmi2) {
    sqlite3_stmt *stmt = null;
    char *result = null;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, null);
    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(db));
        return null;
    }

    sqlite3_bind_text(stmt, 1, wmi3, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, wmi2, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *txt = sqlite3_column_text(stmt, 0);
        if (txt != null) {
            result = strdup((const char *)txt);
        }
    } else if (rc != SQLITE_DONE) {
        log_msg(LOG_ERROR, "sqlite3_step failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return result;
}

char * ISO3779_region(final ISO3779 *decoder) {
    final char * vin = ad_buffer_to_ascii(decoder->vin);
    if ( ad_buffer_alphabet_compare(vin,"A","H") ) {
        return strdup("Africa");
    }
    if ( ad_buffer_alphabet_compare(vin,"J","R") ) {
        return strdup("Asia");
    }
    if ( ad_buffer_alphabet_compare(vin,"S","Z") ) {
        return strdup("Europe");
    }
    if ( ad_buffer_alphabet_compare(vin,"1","5") ) {
        return strdup("North America");
    }
    if ( ad_buffer_alphabet_compare(vin,"6","7") ) {
        return strdup("Oceania");
    }
    if ( ad_buffer_alphabet_compare(vin,"8","9") ) {
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
    printf("    vin: %s\n", ad_buffer_to_ascii(decoder->vin));
    printf("}\n");
}

void ISO3779_free(ISO3779 *decoder) {
    ad_buffer_free(decoder->vin);
    decoder->vin = null;
    AD_PTR_FREE(decoder->country)
    AD_PTR_FREE(decoder->manufacturer)
    decoder->year = -1;
    decoder->wmi = null;
    decoder->vds = null;
    decoder->vis = null;
    free(decoder);
}

char * ISO3779_country(final ISO3779 *decoder) {
    if (decoder == null || decoder->wmi == null) {
        return strdup("Unassigned");
    }

    sqlite3 *db = ISO3779_db_open();
    if (db == null) {
        return strdup("Unassigned");
    }

    char wmi3[4];
    char wmi2[3];

    wmi3[0] = decoder->wmi[0];
    wmi3[1] = decoder->wmi[1];
    wmi3[2] = decoder->wmi[2];
    wmi3[3] = 0;

    wmi2[0] = decoder->wmi[0];
    wmi2[1] = decoder->wmi[1];
    wmi2[2] = 0;

    const char *sql =
        "SELECT c.name "
        "FROM vpic_wmi w "
        "LEFT JOIN vpic_country c ON c.id = w.countryid "
        "WHERE w.wmi = ? OR w.wmi = ? "
        "ORDER BY CASE WHEN w.wmi = ? THEN 0 ELSE 1 END "
        "LIMIT 1;";

    sqlite3_stmt *stmt = null;
    char *result = null;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, null);
    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("Unassigned");
    }

    sqlite3_bind_text(stmt, 1, wmi3, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, wmi2, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, wmi3, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *txt = sqlite3_column_text(stmt, 0);
        if (txt != null) {
            result = strdup((const char *)txt);
        }
    } else if (rc != SQLITE_DONE) {
        log_msg(LOG_ERROR, "sqlite3_step failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return result != null ? result : strdup("Unassigned");
}

char * ISO3779_manufacturer(final ISO3779 *decoder) {
    if (decoder == null || decoder->wmi == null) {
        return strdup("Unknown manufacturer");
    }

    sqlite3 *db = ISO3779_db_open();
    if (db == null) {
        return strdup("Unknown manufacturer");
    }

    char wmi3[4];
    char wmi2[3];

    wmi3[0] = decoder->wmi[0];
    wmi3[1] = decoder->wmi[1];
    wmi3[2] = decoder->wmi[2];
    wmi3[3] = 0;

    wmi2[0] = decoder->wmi[0];
    wmi2[1] = decoder->wmi[1];
    wmi2[2] = 0;

    const char *sql =
        "SELECT m.name "
        "FROM vpic_wmi w "
        "LEFT JOIN vpic_manufacturer m ON m.id = w.manufacturerid "
        "WHERE w.wmi = ? OR w.wmi = ? "
        "ORDER BY CASE WHEN w.wmi = ? THEN 0 ELSE 1 END "
        "LIMIT 1;";

    sqlite3_stmt *stmt = null;
    char *result = null;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, null);
    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return strdup("Unknown manufacturer");
    }

    sqlite3_bind_text(stmt, 1, wmi3, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, wmi2, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, wmi3, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *txt = sqlite3_column_text(stmt, 0);
        if (txt != null) {
            result = strdup((const char *)txt);
        }
    } else if (rc != SQLITE_DONE) {
        log_msg(LOG_ERROR, "sqlite3_step failed: %s", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return result != null ? result : strdup("Unknown manufacturer");
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
    decoder->vin = ad_buffer_copy(vin);
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