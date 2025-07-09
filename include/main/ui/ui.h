#ifndef __UI_H
#define __UI_H

#define _GNU_SOURCE
#include <stdio.h>
#include "ui/gtk_wrapper.h"
#include "libautodiag/log.h"
#include "libautodiag/thread.h"
#include "libautodiag/com/obd/obd.h"
#include "error_feedback.h"
#include "libprog/config.h"

/**
 * Perform operation such as lock release on obd interface on thread cancel event
 */
static void obd_thread_cleanup_routine(void *arg) {
    final VehicleIFace* iface = config.ephemere.iface;
    log_msg(LOG_INFO, "clean up execution");
    if ( iface != null ) {
        viface_unlock(iface);
    }
}

#endif
