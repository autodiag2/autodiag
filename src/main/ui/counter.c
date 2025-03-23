#include "ui/counter.h"

/**
 * Due to the GtkProgressBar implementation if you allocate a width requested
 * less that the required to show the text, the widget will be resized so that the text
 * could be displayed. Eg a progress bar with the text "16000 r/min" will have a greater size
 * than "0 r/min". To get around this just increase the width of the bar.
 */
gboolean counter_draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data) {
    GtkStyleContext *context = gtk_widget_get_style_context (widget);

    guint height = gtk_widget_get_allocated_height (widget);
    guint width = gtk_widget_get_allocated_width (widget);
    gtk_render_background (context, cr, 0, 0, width, height);
    
    int text_height;
    {
        const char *text = gtk_progress_bar_get_text((GtkProgressBar*)widget);
        cairo_text_extents_t extents;
        cairo_scaled_font_text_extents (cairo_get_scaled_font(cr),
                                    text,
                                    &extents);         
        cairo_save (cr);
        GtkStateFlags state = gtk_widget_get_state_flags (widget);
        GdkRGBA rgba;
        gtk_style_context_get_color (context, state, &rgba);
        gdk_cairo_set_source_rgba (cr, &rgba);

        text_height = extents.height;
        int x = (width - extents.width)/2,y = extents.height;
        
        cairo_move_to(cr,x,y);
        cairo_show_text(cr,text);      
        cairo_restore (cr);
    }
    cairo_translate(cr,0,text_height);

    char * img_path;
    final char * data_dir = config_get_in_data_folder_safe("media");
    asprintf(&img_path, "%s" PATH_FOLDER_DELIM "%s", data_dir, (char*)g_object_get_data(G_OBJECT(widget),COUNTER_REL_FILE_PATH_KEY));
    cairo_surface_t * surface = cairo_image_surface_create_from_png(img_path);
    free(data_dir);
    free(img_path);
    int img_w = cairo_image_surface_get_width(surface);
    int img_h = cairo_image_surface_get_height(surface);
    int h_height = height - text_height;
    double scale_x = 1 + (width - img_w * 1.0) / ( img_w * 1.0 );
    double scale_y = 1 + (h_height - img_h * 1.0) / ( img_h * 1.0 );
    double scale = min(scale_x,scale_y);
    int new_img_w = img_w * scale;
    int new_img_h = img_h * scale;
    int img_offset_x = (width - new_img_w) / 2;
    int img_offset_y = (h_height - new_img_h) / 2;

    cairo_translate(cr,img_offset_x,img_offset_y);
    cairo_scale(cr,scale,scale);
    cairo_set_source_surface (cr,surface,0,0);
    cairo_paint(cr);

    {
        int throttle_length_on_picture = *((int*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_LENGTH_ON_PICTURE));
        double throttle_r = *((double*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_R)), 
               throttle_g = *((double*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_G)), 
               throttle_b = *((double*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_B)),
               throttle_a = *((double*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_A));
        double throttle_width = *((double*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_WIDTH));

        int throttle_center_offset_on_picture_x = *((int*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_OFFSET_ON_PICT_X)), 
            throttle_center_offset_on_picture_y = *((int*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_OFFSET_ON_PICT_Y));
        
        if ( throttle_center_offset_on_picture_x == -1 ) {
            throttle_center_offset_on_picture_x = img_w / 2;
        } 
        if ( throttle_center_offset_on_picture_y == -1 ) {
            throttle_center_offset_on_picture_y = 0;
        }

        int throttle_screen_offset_on_picture_x = throttle_center_offset_on_picture_x - throttle_length_on_picture,
            throttle_screen_offset_on_picture_y = img_h - throttle_center_offset_on_picture_y - throttle_length_on_picture;

        int throttle_screen_width = throttle_length_on_picture * 2,
            throttle_screen_height = throttle_length_on_picture;

        int throttle_center_x = throttle_screen_width/2,
            throttle_center_y = throttle_screen_height;

        double p = gtk_progress_bar_get_fraction((GtkProgressBar*)widget);
        double rad = (1-p) * G_PI;
        int throttle_extreme_x = throttle_screen_width * (1 + cos(rad)) / 2.0,
            throttle_extreme_y = throttle_screen_height * (1 - sin(rad));

        /*
        printf("img screen=(%d,%d) throttle screen=(%d,%d) (%d,%d) (%d,%d)\n", 
            img_w,img_h,
            throttle_screen_width,throttle_screen_height,
            throttle_center_x, throttle_center_y,
            throttle_extreme_x, throttle_extreme_y);
        */
        cairo_translate(cr,throttle_screen_offset_on_picture_x,throttle_screen_offset_on_picture_y);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_width(cr, throttle_width);
        cairo_set_source_rgba (cr, throttle_r, throttle_g, throttle_b, throttle_a);
        cairo_move_to(cr,throttle_center_x,throttle_center_y);
        cairo_line_to(cr,throttle_extreme_x,throttle_extreme_y);
        cairo_stroke(cr);
    }
    return TRUE;
}
void counter_destroy_progress_bar_allocations(gpointer data) {
    if ( data != null ) {
        free(data);
    }
}

double counter_throttle_calculate_angle(double x, double y) {
    //return atan2(y - CENTER_Y, x - CENTER_X);
    return 0.5;
}
gboolean counter_on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->button == 1) {
        double angle = counter_throttle_calculate_angle(event->x, event->y);
        gtk_progress_bar_set_fraction((GtkProgressBar*)widget, 1 - angle / G_PI);
        gtk_widget_queue_draw(widget);
        g_object_set_data_full (G_OBJECT(widget), COUNTER_KEY_THROTTLE_DRAGGING,
            intdup(1), 
            &counter_destroy_progress_bar_allocations
        );
        gtk_widget_queue_draw(widget);
    }
    return TRUE;
}
gboolean counter_on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    g_object_set_data_full (G_OBJECT(widget), COUNTER_KEY_THROTTLE_DRAGGING,
        intdup(0), 
        &counter_destroy_progress_bar_allocations
    );
    return TRUE;
}
gboolean counter_on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    int dragging = *((int*)g_object_get_data(G_OBJECT(widget),COUNTER_KEY_THROTTLE_DRAGGING));
    if (dragging) {
        double angle = counter_throttle_calculate_angle(event->x, event->y);
        gtk_progress_bar_set_fraction((GtkProgressBar*)widget, 1 - angle / G_PI);
        gtk_widget_queue_draw(widget);
    }
    return TRUE;
}

GtkProgressBar* counter_init(GtkProgressBar* bar) {
    return counter_init_with(bar,"counter_85_2_255_0_0_255.png");
}
GtkProgressBar* counter_init_with(GtkProgressBar* bar, char *pngName) {
    return counter_init_modifiable(bar,pngName, false);
}
GtkProgressBar* counter_init_modifiable(GtkProgressBar* bar, char *pngName, bool isModifiable) {
    assert(bar != null);
    g_object_set_data_full (G_OBJECT(bar), COUNTER_REL_FILE_PATH_KEY,
                            strdup(pngName), 
                            &counter_destroy_progress_bar_allocations
                            );
    char * ptr = pngName;
    int throttle_length_on_pict = 78;
    int throttle_width = 2;
    int throttle_r = 255, throttle_g = 0, throttle_b = 0, throttle_a = 255;
    int throttle_offset_on_pict_x = -1, throttle_offset_on_pict_y = -1;
    ptr = strstr(ptr,"_");
    if ( ptr != null ) {
        sscanf(ptr+1,"%d",&throttle_length_on_pict);
        ptr = strstr(ptr+1,"_");
        if ( ptr != null ) {
            sscanf(ptr+1,"%d",&throttle_width);
            ptr = strstr(ptr+1,"_");
            if ( ptr != null ) {
                sscanf(ptr+1,"%d",&throttle_r);
                ptr = strstr(ptr+1,"_");
                if ( ptr != null ) {
                    sscanf(ptr+1,"%d",&throttle_g);
                    ptr = strstr(ptr+1,"_");
                    if ( ptr != null ) {
                        sscanf(ptr+1,"%d",&throttle_b);
                        ptr = strstr(ptr+1,"_");
                        if ( ptr != null ) {
                            sscanf(ptr+1,"%d",&throttle_a);
                            ptr = strstr(ptr+1,"_");
                            if ( ptr != null ) {
                                sscanf(ptr+1,"%d",&throttle_offset_on_pict_x);
                                ptr = strstr(ptr+1,"_");
                                if ( ptr != null ) {
                                    sscanf(ptr+1,"%d",&throttle_offset_on_pict_y);
                                }
                            }
                        }
                    }
                }
            }            
        }
    }
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_LENGTH_ON_PICTURE,
                        intdup(throttle_length_on_pict), 
                        &counter_destroy_progress_bar_allocations
                        );
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_WIDTH,
                            doubledup(throttle_width * 1.0), 
                            &counter_destroy_progress_bar_allocations
                            );
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_R,
                            doubledup(throttle_r/255.0), 
                            &counter_destroy_progress_bar_allocations
                            );
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_G,
                            doubledup(throttle_g/255.0), 
                            &counter_destroy_progress_bar_allocations
                            );
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_B,
                            doubledup(throttle_b/255.0), 
                            &counter_destroy_progress_bar_allocations
                            );
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_A,
                            doubledup(throttle_a/255.0), 
                            &counter_destroy_progress_bar_allocations
                            );
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_OFFSET_ON_PICT_X,
                            intdup(throttle_offset_on_pict_x), 
                            &counter_destroy_progress_bar_allocations
                            );
    g_object_set_data_full (G_OBJECT(bar), COUNTER_KEY_THROTTLE_OFFSET_ON_PICT_Y,
                            intdup(throttle_offset_on_pict_y), 
                            &counter_destroy_progress_bar_allocations
                            );
    g_signal_connect (G_OBJECT (bar), "draw", G_CALLBACK (counter_draw_callback), NULL);  
    if ( isModifiable ) {
        g_signal_connect(G_OBJECT (bar), "button-press-event", G_CALLBACK(counter_on_button_press), NULL);
        g_signal_connect(G_OBJECT (bar), "button-release-event", G_CALLBACK(counter_on_button_release), NULL);
        g_signal_connect(G_OBJECT (bar), "motion-notify-event", G_CALLBACK(counter_on_motion_notify), NULL);
        gtk_widget_add_events(GTK_WIDGET(bar), GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    }
    return bar;
}
