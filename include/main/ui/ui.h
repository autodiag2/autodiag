#ifndef __UI_H
#define __UI_H

#define _GNU_SOURCE
#include <stdio.h>
#include "libprog/ui/gtk_wrapper.h"
#include "libautodiag/log.h"
#include "libautodiag/thread.h"
#include "libautodiag/com/obd/obd.h"
#include "ui/widget/error_feedback.h"
#include "libprog/config.h"

/**
 * Perform operation such as lock release on obd interface on thread cancel event
 */
static void thread_viface_cleanup_routine(void *arg) {
    final VehicleIFace* iface = config.ephemere.iface;
    assert(iface != null);
    log_msg(LOG_INFO, "clean up execution");
    if ( iface->state == VIFaceState_READY ) {
        viface_unlock(iface);
    }
}

#endif
