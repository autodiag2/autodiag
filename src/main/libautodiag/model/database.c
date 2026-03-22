#include "libautodiag/model/database.h"
#include "sqlite3.h"

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

    return strcasecmp(a, b) == 0;
}

static bool db_is_empty_str(const char *s) {
    return s == null || s[0] == 0;
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

static bool db_vehicle_add_ecu(Vehicle *vehicle, const char *manufacturer, const char *model, const char *type) {
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

    if ( type != null ) {
        ecu->type = strdup(type);
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

static bool db_dtc_ecu_matches_filter(
    const char *ecu_manufacturer,
    const char *ecu_model,
    const Vehicle *filter
) {
    if (filter == null || filter->ecus == null || filter->ecus->size == 0) {
        return true;
    }

    if (db_is_empty_str(ecu_manufacturer) && db_is_empty_str(ecu_model)) {
        return true;
    }

    for (int i = 0; i < filter->ecus->size; i++) {
        final ad_object_ECU *ecu = filter->ecus->list[i];
        if (ecu == null) {
            continue;
        }

        if (!db_is_empty_str(ecu_manufacturer) && !db_str_eq(ecu_manufacturer, ecu->manufacturer)) {
            continue;
        }

        if (!db_is_empty_str(ecu_model) && !db_str_eq(ecu_model, ecu->model)) {
            continue;
        }

        return true;
    }

    return false;
}

static Vehicle *db_vehicle_from_ecu_row(
    const char *ecu_manufacturer,
    const char *ecu_model,
    const char *ecu_type
) {
    Vehicle *vehicle = vehicle_new();
    if (vehicle == null) {
        return null;
    }

    if (!db_vehicle_add_ecu(vehicle, ecu_manufacturer, ecu_model, ecu_type)) {
        vehicle_free(vehicle);
        return null;
    }

    if (vehicle->ecus == null || vehicle->ecus->size == 0) {
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
        "    eum.name AS ecu_manufacturer, "
        "    ecu.model AS ecu_model, "
        "    ecu.type AS ecu_type "
        "FROM ad_dtc d "
        "LEFT JOIN ad_ecu ecu "
        "    ON ecu.id = d.ecu_id "
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
        const char *ecu_manufacturer = (const char *)sqlite3_column_text(stmt, 7);
        const char *ecu_model = (const char *)sqlite3_column_text(stmt, 8);
        const char *ecu_type = (const char*)sqlite3_column_text(stmt, 9);

        log_msg(
            LOG_DEBUG,
            "Found row : %s %s %d %s %s %s %s",
            definition,
            description,
            mil,
            detection_condition,
            causes,
            repairs,
            evidence
        );
        log_msg(
            LOG_DEBUG,
            " ecu filter: %s %s",
            ecu_manufacturer != null ? ecu_manufacturer : "",
            ecu_model != null ? ecu_model : ""
        );

        (void)description;
        (void)mil;
        (void)detection_condition;
        (void)causes;
        (void)evidence;

        if (!db_dtc_ecu_matches_filter(ecu_manufacturer, ecu_model, filter)) {
            log_msg(LOG_DEBUG, "entry filtered");
            continue;
        }

        Vehicle *scope_vehicle = db_vehicle_from_ecu_row(
            ecu_manufacturer,
            ecu_model,
            ecu_type
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