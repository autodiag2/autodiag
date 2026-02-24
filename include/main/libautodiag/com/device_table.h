#ifndef __COM_DEVICE_TABLE_H
#define __COM_DEVICE_TABLE_H

#include "libautodiag/dirent.h"
#include "libautodiag/log.h"
#include "libautodiag/com/device.h"

#ifdef OS_WINDOWS
#   include "libautodiag/windows.h"
#   include <setupapi.h>
#   include <initguid.h>
#   include <devguid.h>
#   include <stdio.h>
#   include <string.h>
#   define WINDOWS_SERIAL_EMU_PIPE_PREFIX "elm327sim_"
#endif

AD_LIST_H(Device)

OBJECT_H(DeviceTable,
    list_Device * list;
    int selected_index;
)
#define DEVICE_TABLE_NO_SELECTED -1

void device_table_close_selected(object_DeviceTable * table);
/**
 * Fill the device table with currently detected COM ports/pipes/network.
 */
void device_table_fill(object_DeviceTable * table);
void device_table_free(object_DeviceTable * table);
/**
 * @return NULL if not found
 */
Device * device_table_find_by_location(object_DeviceTable * table, final char * location);
Device * device_table_add_if_not_in_by_location(object_DeviceTable * table, char * location, DEVICE_TYPE type);

bool device_table_remove(object_DeviceTable * table, final Device * element);
void device_table_set_to_undetected(object_DeviceTable * table);
void device_table_remove_undetected(object_DeviceTable * table, bool except_network);
/**
 * Get the selected Device (port on which we are currently working) or NULL if no port currently selected.
 */
Device * device_table_get_selected(object_DeviceTable * table);
void device_table_set_selected_by_location(object_DeviceTable * table, char *location);
/**
 * Update device in the table.
 */
bool device_table_update_device(object_DeviceTable * table, Device * old, Device * new);

#endif
