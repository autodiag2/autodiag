#include "libautodiag/model/database.h"
#include "sqlite3.h"

ad_list_Vehicle database = { .list = null, .size = 0 };

static int db_year_from_text(const char *years) {
    if (years == null || years[0] == 0) {
        return 0;
    }

    while (*years != 0 && (*years < '0' || *years > '9')) {
        years++;
    }

    if (years[0] == 0) {
        return 0;
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
    return strcmp(a, b) == 0;
}

static Vehicle *db_vehicle_find_loaded(const char *manufacturer, const char *model, int year) {
    for (int i = 0; i < database.size; i++) {
        Vehicle *vehicle = database.list[i];
        if (vehicle == null) {
            continue;
        }
        if (!db_str_eq(vehicle->manufacturer, manufacturer)) {
            continue;
        }
        if (!db_str_eq(vehicle->model, model)) {
            continue;
        }
        if (vehicle->year != year) {
            continue;
        }
        return vehicle;
    }
    return null;
}

static bool db_vehicle_has_ecu(Vehicle *vehicle, const char *manufacturer, const char *model) {
    if (vehicle == null || vehicle->ecus == null) {
        return false;
    }

    for (int i = 0; i < vehicle->ecus->size; i++) {
        ECU *ecu = vehicle->ecus->list[i];
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

    if ((manufacturer == null || manufacturer[0] == 0) && (model == null || model[0] == 0)) {
        return true;
    }

    if (db_vehicle_has_ecu(vehicle, manufacturer, model)) {
        return true;
    }

    ECU *ecu = vehicle_ecu_new();
    if (ecu == null) {
        log_msg(LOG_ERROR, "ecu_new failed");
        return false;
    }

    if (manufacturer != null) {
        ecu->manufacturer = strdup(manufacturer);
    }
    if (model != null) {
        ecu->model = strdup(model);
    }

    ad_list_ECU_append(vehicle->ecus, ecu);
    return true;
}

static Vehicle *db_vehicle_get_or_create(
    const char *manufacturer,
    const char *model,
    const char *years,
    const char *engine_manufacturer,
    const char *engine_model
) {
    int year = db_year_from_text(years);

    Vehicle *vehicle = db_vehicle_find_loaded(manufacturer, model, year);
    if (vehicle != null) {
        if ((vehicle->engine == null || vehicle->engine[0] == 0) && engine_model != null) {
            vehicle->engine = strdup(engine_model);
        }
        if ((vehicle->engine_manufacturer == null || vehicle->engine_manufacturer[0] == 0) && engine_manufacturer != null) {
            vehicle->engine_manufacturer = strdup(engine_manufacturer);
        }
        return vehicle;
    }

    vehicle = vehicle_new();
    if (vehicle == null) {
        log_msg(LOG_ERROR, "vehicle_new failed");
        return null;
    }

    if (manufacturer != null) {
        vehicle->manufacturer = strdup(manufacturer);
    }
    if (model != null) {
        vehicle->model = strdup(model);
    }
    vehicle->year = year;
    if (engine_model != null) {
        vehicle->engine = strdup(engine_model);
    }
    if (engine_manufacturer != null) {
        vehicle->engine_manufacturer = strdup(engine_manufacturer);
    }

    ad_list_Vehicle_append(&database, vehicle);
    return vehicle;
}

static bool db_vehicle_load_in_memory_sqlite_file(const char *sqlite_path) {
    assert(sqlite_path != null);

    sqlite3 *db = null;
    sqlite3_stmt *stmt = null;

    int rc = sqlite3_open(sqlite_path, &db);
    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_open failed for %s: %s", sqlite_path, db != null ? sqlite3_errmsg(db) : "unknown error");
        if (db != null) {
            sqlite3_close(db);
        }
        return false;
    }

    const char *sql =
        "SELECT "
        "    vm.name AS vehicle_manufacturer, "
        "    v.model AS vehicle_model, "
        "    v.years AS years, "
        "    em.name AS engine_manufacturer, "
        "    eng.model AS engine_model, "
        "    eum.name AS ecu_manufacturer, "
        "    ecu.model AS ecu_model "
        "FROM ad_vehicle v "
        "LEFT JOIN ad_manufacturer vm "
        "    ON vm.id = v.manufacturer_id "
        "LEFT JOIN ad_vehicle_engine_link vel "
        "    ON vel.vehicle_id = v.id "
        "LEFT JOIN ad_engine eng "
        "    ON eng.id = vel.engine_id "
        "LEFT JOIN ad_manufacturer em "
        "    ON em.id = eng.manufacturer_id "
        "LEFT JOIN ad_vehicle_ecu_link vecul "
        "    ON vecul.vehicle_id = v.id "
        "LEFT JOIN ad_ecu ecu "
        "    ON ecu.id = vecul.ecu_id "
        "LEFT JOIN ad_manufacturer eum "
        "    ON eum.id = ecu.manufacturer_id "
        "ORDER BY vm.name, v.model, v.years, eng.model, ecu.model;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, null);
    if (rc != SQLITE_OK) {
        log_msg(LOG_ERROR, "sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *vehicle_manufacturer = (const char *)sqlite3_column_text(stmt, 0);
        const char *vehicle_model = (const char *)sqlite3_column_text(stmt, 1);
        const char *years = (const char *)sqlite3_column_text(stmt, 2);
        const char *engine_manufacturer = (const char *)sqlite3_column_text(stmt, 3);
        const char *engine_model = (const char *)sqlite3_column_text(stmt, 4);
        const char *ecu_manufacturer = (const char *)sqlite3_column_text(stmt, 5);
        const char *ecu_model = (const char *)sqlite3_column_text(stmt, 6);

        Vehicle *vehicle = db_vehicle_get_or_create(
            vehicle_manufacturer,
            vehicle_model,
            years,
            engine_manufacturer,
            engine_model
        );
        if (vehicle == null) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }

        if (!db_vehicle_add_ecu(vehicle, ecu_manufacturer, ecu_model)) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }
    }

    if (rc != SQLITE_DONE) {
        log_msg(LOG_ERROR, "sqlite3_step failed: %s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

void db_vehicle_load_in_memory() {
    ad_list_Vehicle_empty(&database);

    char *sqlite_path = installation_folder_resolve("data/ad_database.sqlite");
    if (sqlite_path == null) {
        log_msg(LOG_ERROR, "Cannot resolve sqlite database path");
        return;
    }

    if (!db_vehicle_load_in_memory_sqlite_file(sqlite_path)) {
        log_msg(LOG_ERROR, "Failed to load vehicles from sqlite database: %s", sqlite_path);
    }

    free(sqlite_path);
}