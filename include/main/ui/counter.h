#ifndef __UI_COUNTER_H
#define __UI_COUNTER_H

#include "ui/gtk.h"
#include "ui/config.h"
#include "lib/math.h"

GtkProgressBar* counter_init(GtkProgressBar* bar);
/**
 * pngName encoded as: compteur_C_W_R_G_B_A_X_Y.png
 * C: throttle length in pixels on the image of the counter
 * W: width in pixels of the throttle on the image of the counter
 * R: Red composite of throttle color [0;255]
 * G: Red composite of throttle color [0;255] 
 * B: Red composite of throttle color [0;255]
 * A: Red composite of throttle color [0;255]
 * X: X-offset in pixels off the throttle center on the image of the counter with (0,0) at bottom left
 * Y: Y-offset in pixels off the throttle center on the image of the counter with (0,0) at bottom left
 */
GtkProgressBar* counter_init_with(GtkProgressBar* bar, char *pngName);

#define COUNTER_REL_FILE_PATH_KEY "counter_rel_file_path"
#define COUNTER_KEY_THROTTLE_LENGTH_ON_PICTURE "counter_throttle_length_on_picture"
#define COUNTER_KEY_THROTTLE_R "counter_key_throttle_r"
#define COUNTER_KEY_THROTTLE_G "counter_key_throttle_g"
#define COUNTER_KEY_THROTTLE_B "counter_key_throttle_b"
#define COUNTER_KEY_THROTTLE_A "counter_key_throttle_a"
#define COUNTER_KEY_THROTTLE_WIDTH "counter_key_throttle_width"
#define COUNTER_KEY_THROTTLE_OFFSET_ON_PICT_X "counter_key_throttle_offset_on_pict_x"
#define COUNTER_KEY_THROTTLE_OFFSET_ON_PICT_Y "counter_key_throttle_offset_on_pict_y"
#endif
