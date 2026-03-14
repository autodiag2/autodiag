#include "libautodiag/model/database.h"
#include "sqlite3.h"

static int db_year_from_text(const char *years) {
    if (years == null || years[0] == 0) {
        return VEHICLE_YEAR_EMPTY;
    }

    while (*years != 0 && (*years < '0' || *years > '9')) {
        years++;
    }

    if (*years == 0) {
        return VEHICLE_YEAR_EMPTY;
    }

    return atoi(years);
}

static bool db_str_eq(const char *a, const char *b) {
    if (a == null && b == null) {
        return true;
    }

    if (a == null || b == null) {
        return false;
    }

    if (a[0] == 0 && b[0] == 0) {
        return true;
    }

    return strcmp(a, b) == 0;
}

static bool db_vehicle_has_ecu(Vehicle *vehicle, const char *manufacturer, const char *model) {
    if (vehicle == null) {
        return false;
    }

    if (vehicle->ecus == null || vehicle->ecus->size == 0) {
        return false;
    }

    for (int i = 0; i < vehicle->ecus->size; i++) {
        ad_object_ECU *ecu = vehicle->ecus->list[i];

        if (ecu == null) {
            continue;
        }

        if (!db_str_eq(ecu->manufacturer, manufacturer)) {
            continue;
        }

        if (!db_str_eq(ecu->model, model)) {
            continue;
        }

        return true;
    }

    return false;
}

static bool db_vehicle_add_ecu(Vehicle *vehicle, const char *manufacturer, const char *model) {
    if (vehicle == null) {
        return false;
    }

    if ((manufacturer == null || manufacturer[0] == 0) &&
        (model == null || model[0] == 0)) {
        return true;
    }

    if (db_vehicle_has_ecu(vehicle, manufacturer, model)) {
        return true;
    }

    ad_object_ECU *ecu = ad_object_ECU_new();
    if (ecu == null) {
        log_msg(LOG_ERROR, "ad_object_ECU_new failed");
        return false;
    }

    if (manufacturer != null) {
        ecu->manufacturer = strdup(manufacturer);
    }

    if (model != null) {
        ecu->model = strdup(model);
    }

    if (vehicle->ecus == null) {
        vehicle->ecus = ad_list_ad_object_ECU_new();
        if (vehicle->ecus == null) {
            log_msg(LOG_ERROR, "ad_list_ad_object_ECU_new failed");
            ad_object_ECU_free(ecu);
            return false;
        }
    }

    ad_list_ad_object_ECU_append(vehicle->ecus, ecu);

    return true;
}
static void db_replace_str(char **dst, const char *src) {
    if (dst == null) {
        return;
    }
    if (*dst != null) {
        free(*dst);
        *dst = null;
    }
    if (src != null) {
        *dst = strdup(src);
    }
}

static bool db_is_empty_str(const char *s) {
    return s == null || s[0] == 0;
}

static bool db_scope_field_matches_filter_field(const char *scope_value, const char *filter_value) {
    if (db_is_empty_str(filter_value)) {
        return true;
    }
    if (db_is_empty_str(scope_value)) {
        return true;
    }
    return strcmp(scope_value, filter_value) == 0;
}

static bool db_year_text_matches_filter_year(const char *years, int year) {
    char year_str[32];

    if (year == VEHICLE_YEAR_EMPTY || year <= 0) {
        return true;
    }
    if (db_is_empty_str(years)) {
        return true;
    }

    snprintf(year_str, sizeof(year_str), "%d", year);

    if (strcmp(years, "any") == 0) {
        return true;
    }
    if (strcmp(years, year_str) == 0) {
        return true;
    }
    if (strstr(years, year_str) != null) {
        return true;
    }

    return false;
}

static bool db_scope_ecu_matches_filter(
    const char *scope_ecu_manufacturer,
    const char *scope_ecu_model,
    const Vehicle *filter
) {
    if (filter == null || filter->ecus == null || filter->ecus->size == 0) {
        return true;
    }

    if (db_is_empty_str(scope_ecu_manufacturer) && db_is_empty_str(scope_ecu_model)) {
        return true;
    }

    for (int i = 0; i < filter->ecus->size; i++) {
        final ad_object_ECU *ecu = filter->ecus->list[i];
        if (ecu == null) {
            continue;
        }

        if (!db_scope_field_matches_filter_field(scope_ecu_manufacturer, ecu->manufacturer)) {
            continue;
        }
        if (!db_scope_field_matches_filter_field(scope_ecu_model, ecu->model)) {
            continue;
        }

        return true;
    }

    return false;
}

static int db_scope_match_score(
    const char *vehicle_manufacturer,
    const char *vehicle_model,
    const char *years,
    const char *engine_manufacturer,
    const char *engine_model,
    const char *ecu_manufacturer,
    const char *ecu_model
) {
    int score = 0;

    if (!db_is_empty_str(vehicle_manufacturer)) {
        score++;
    }
    if (!db_is_empty_str(vehicle_model)) {
        score++;
    }
    if (!db_is_empty_str(years)) {
        score++;
    }
    if (!db_is_empty_str(engine_manufacturer)) {
        score++;
    }
    if (!db_is_empty_str(engine_model)) {
        score++;
    }
    if (!db_is_empty_str(ecu_manufacturer)) {
        score++;
    }
    if (!db_is_empty_str(ecu_model)) {
        score++;
    }

    return score;
}

static bool db_scope_matches_filter(
    const char *vehicle_manufacturer,
    const char *vehicle_model,
    const char *years,
    const char *engine_manufacturer,
    const char *engine_model,
    const char *ecu_manufacturer,
    const char *ecu_model,
    const Vehicle *filter
) {
    if (filter == null) {
        return true;
    }

    if (!db_scope_field_matches_filter_field(vehicle_manufacturer, filter->manufacturer)) {
        return false;
    }
    if (!db_scope_field_matches_filter_field(vehicle_model, filter->model)) {
        return false;
    }
    if (!db_year_text_matches_filter_year(years, filter->year)) {
        return false;
    }
    if (!db_scope_field_matches_filter_field(engine_manufacturer, filter->engine_manufacturer)) {
        return false;
    }
    if (!db_scope_field_matches_filter_field(engine_model, filter->engine)) {
        return false;
    }
    if (!db_scope_ecu_matches_filter(ecu_manufacturer, ecu_model, filter)) {
        return false;
    }

    return true;
}
static Vehicle *db_vehicle_from_scope_row(
    const char *vehicle_manufacturer,
    const char *vehicle_model,
    const char *years,
    const char *engine_manufacturer,
    const char *engine_model,
    const char *ecu_manufacturer,
    const char *ecu_model
) {
    Vehicle *vehicle = vehicle_new();
    if (vehicle == null) {
        return null;
    }

    if (vehicle_manufacturer != null) {
        vehicle->manufacturer = strdup(vehicle_manufacturer);
    }
    if (vehicle_model != null) {
        vehicle->model = strdup(vehicle_model);
    }

    if (years != null && years[0] != 0) {
        vehicle->year = db_year_from_text(years);
    } else {
        vehicle->year = VEHICLE_YEAR_EMPTY;
    }

    if (engine_manufacturer != null) {
        vehicle->engine_manufacturer = strdup(engine_manufacturer);
    }
    if (engine_model != null) {
        vehicle->engine = strdup(engine_model);
    }

    if ((ecu_manufacturer != null && ecu_manufacturer[0] != 0)
    ||  (ecu_model != null && ecu_model[0] != 0)) {
        if (!db_vehicle_add_ecu(vehicle, ecu_manufacturer, ecu_model)) {
            vehicle_free(vehicle);
            return null;
        }
    }

    if (vehicle->manufacturer == null
    &&  vehicle->model == null
    &&  vehicle->year == VEHICLE_YEAR_EMPTY
    &&  vehicle->engine_manufacturer == null
    &&  vehicle->engine == null
    &&  (vehicle->ecus == null || vehicle->ecus->size == 0)) {
        vehicle_free(vehicle);
        return null;
    }

    return vehicle;
}

static bool db_dtc_description_already_present(
    final ad_list_DTC_DESCRIPTION *list,
    const char *reason,
    const char *solution,
    final Vehicle *vehicle
) {
    if (list == null) {
        return false;
    }

    for (int i = 0; i < list->size; i++) {
        final DTC_DESCRIPTION *desc = &list->list[i];

        if (!db_str_eq(desc->reason, reason)) {
            continue;
        }
        if (!db_str_eq(desc->solution, solution)) {
            continue;
        }

        if (desc->vehicle == null && vehicle == null) {
            return true;
        }
        if (desc->vehicle == null || vehicle == null) {
            continue;
        }

        if (!db_str_eq(desc->vehicle->manufacturer, vehicle->manufacturer)) {
            continue;
        }
        if (!db_str_eq(desc->vehicle->model, vehicle->model)) {
            continue;
        }
        if (desc->vehicle->year != vehicle->year) {
            continue;
        }
        if (!db_str_eq(desc->vehicle->engine_manufacturer, vehicle->engine_manufacturer)) {
            continue;
        }
        if (!db_str_eq(desc->vehicle->engine, vehicle->engine)) {
            continue;
        }

        if ((desc->vehicle->ecus == null || desc->vehicle->ecus->size == 0)
        &&  (vehicle->ecus == null || vehicle->ecus->size == 0)) {
            return true;
        }

        if (desc->vehicle->ecus == null || vehicle->ecus == null) {
            continue;
        }
        if (desc->vehicle->ecus->size != vehicle->ecus->size) {
            continue;
        }

        bool all_ecus_match = true;
        for (int j = 0; j < vehicle->ecus->size; j++) {
            final ad_object_ECU *ecu = vehicle->ecus->list[j];
            if (ecu == null) {
                continue;
            }
            if (!db_vehicle_has_ecu(desc->vehicle, ecu->manufacturer, ecu->model)) {
                all_ecus_match = false;
                break;
            }
        }

        if (all_ecus_match) {
            return true;
        }
    }

    return false;
}
static bool db_dtc_is_generic_scope(const char *vehicle_manufacturer) {
    if (db_str_eq(vehicle_manufacturer, "Generic")) {
        return true;
    }
    if (db_str_eq(vehicle_manufacturer, "saej2012.2002")) {
        return true;
    }
    return false;
}
void ad_dtc_fetch_from_db(final DTC *dtc, final Vehicle *filter) {
    if (dtc == null) {
        return;
    }

    char *dtc_code = dtc->to_string(AD_DTC(dtc));
    if (db_is_empty_str(dtc_code)) {
        log_msg(LOG_WARNING, "ad_dtc_fetch_from_db called with empty dtc code");
        if (dtc_code != null) {
            free(dtc_code);
        }
        return;
    }

    char *sqlite_path = installation_folder_resolve("data/ad_database.sqlite");
    if (sqlite_path == null) {
        log_msg(LOG_ERROR, "Cannot resolve sqlite database path");
        free(dtc_code);
        return;
    }

    sqlite3 *db = null;
    sqlite3_stmt *stmt = null;

    int rc = sqlite3_open(sqlite_path, &db);
    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_open failed for %s: %s", sqlite_path, db != null ? sqlite3_errmsg(db) : "unknown error");
        if (db != null) {
            sqlite3_close(db);
        }
        free(sqlite_path);
        free(dtc_code);
        return;
    }

    log_debug("searching for '%s'", dtc_code);
    const char *sql =
        "SELECT "
        "    d.definition, "
        "    d.description, "
        "    d.mil, "
        "    d.detection_condition, "
        "    d.causes, "
        "    d.repairs, "
        "    d.evidence, "
        "    vm.name AS vehicle_manufacturer, "
        "    v.model AS vehicle_model, "
        "    v.years AS years, "
        "    em.name AS engine_manufacturer, "
        "    eng.model AS engine_model, "
        "    eum.name AS ecu_manufacturer, "
        "    ecu.model AS ecu_model "
        "FROM ad_dtc d "
        "LEFT JOIN ad_dtc_scope_link sl "
        "    ON sl.dtc_id = d.id "
        "LEFT JOIN ad_vehicle v "
        "    ON v.id = sl.vehicle_id "
        "LEFT JOIN ad_manufacturer vm "
        "    ON vm.id = v.manufacturer_id "
        "LEFT JOIN ad_engine eng "
        "    ON eng.id = sl.engine_id "
        "LEFT JOIN ad_manufacturer em "
        "    ON em.id = eng.manufacturer_id "
        "LEFT JOIN ad_ecu ecu "
        "    ON ecu.id = sl.ecu_id "
        "LEFT JOIN ad_manufacturer eum "
        "    ON eum.id = ecu.manufacturer_id "
        "WHERE lower(d.code) = lower(?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, null);
    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        free(sqlite_path);
        free(dtc_code);
        return;
    }

    sqlite3_bind_text(stmt, 1, dtc_code, -1, SQLITE_TRANSIENT);

    if (dtc->description != null) {
        ad_list_DTC_DESCRIPTION_free(dtc->description);
    }
    dtc->description = ad_list_DTC_DESCRIPTION_new();
    if (dtc->description == null) {
        log_msg(LOG_ERROR, "ad_list_DTC_DESCRIPTION_new failed");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        free(sqlite_path);
        free(dtc_code);
        return;
    }

    int matches = 0;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *definition = (const char *)sqlite3_column_text(stmt, 0);
        const char *description = (const char *)sqlite3_column_text(stmt, 1);
        const int mil = sqlite3_column_type(stmt, 2) == SQLITE_NULL ? 0 : sqlite3_column_int(stmt, 2);
        const char *detection_condition = (const char *)sqlite3_column_text(stmt, 3);
        const char *causes = (const char *)sqlite3_column_text(stmt, 4);
        const char *repairs = (const char *)sqlite3_column_text(stmt, 5);
        const char *evidence = (const char *)sqlite3_column_text(stmt, 6);

        const char *vehicle_manufacturer = (const char *)sqlite3_column_text(stmt, 7);
        const char *vehicle_model = (const char *)sqlite3_column_text(stmt, 8);
        const char *years = (const char *)sqlite3_column_text(stmt, 9);
        const char *engine_manufacturer = (const char *)sqlite3_column_text(stmt, 10);
        const char *engine_model = (const char *)sqlite3_column_text(stmt, 11);
        const char *ecu_manufacturer = (const char *)sqlite3_column_text(stmt, 12);
        const char *ecu_model = (const char *)sqlite3_column_text(stmt, 13);

        log_msg(LOG_DEBUG, "Found row : %s %s %d %s %s %s %s", definition, description, mil, detection_condition, causes, repairs, evidence);
        log_msg(LOG_DEBUG, " scope: %s %s %s %s %s %s %s", vehicle_manufacturer, vehicle_model, years, engine_manufacturer, engine_model, ecu_manufacturer, ecu_model);

        (void)description;
        (void)mil;
        (void)detection_condition;
        (void)causes;
        (void)evidence;

        if (!db_dtc_is_generic_scope(vehicle_manufacturer)) {
            if (!db_scope_matches_filter(
                vehicle_manufacturer,
                vehicle_model,
                years,
                engine_manufacturer,
                engine_model,
                ecu_manufacturer,
                ecu_model,
                filter
            )) {
                log_msg(LOG_DEBUG, "entry filtered");
                continue;
            }
        }

        Vehicle *scope_vehicle = db_vehicle_from_scope_row(
            vehicle_manufacturer,
            vehicle_model,
            years,
            engine_manufacturer,
            engine_model,
            ecu_manufacturer,
            ecu_model
        );

        const char *reason = definition;
        const char *solution = repairs;

        if (db_dtc_description_already_present(dtc->description, reason, solution, scope_vehicle)) {
            if (scope_vehicle != null) {
                vehicle_free(scope_vehicle);
            }
            continue;
        }

        DTC_DESCRIPTION *desc = dtc_description_new();
        if (desc == null) {
            if (scope_vehicle != null) {
                vehicle_free(scope_vehicle);
            }
            continue;
        }

        if (reason != null) {
            desc->reason = strdup(reason);
        }
        if (solution != null) {
            desc->solution = strdup(solution);
        }
        desc->vehicle = scope_vehicle;

        ad_list_DTC_DESCRIPTION_append(dtc->description, desc);
        matches++;
    }

    if (rc != SQLITE_DONE) {
        log_msg(LOG_ERROR, "sqlite3_step failed: %s", sqlite3_errmsg(db));
    } else if (matches == 0) {
        log_msg(LOG_DEBUG, "No DTC entry found in database for code=%s with current filter", dtc_code);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(sqlite_path);
    free(dtc_code);
}