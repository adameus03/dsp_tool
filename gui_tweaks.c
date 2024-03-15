#include "gui_tweaks.h"

/* No-op to prevent @w from propagating "scroll" events it receives.
 */
void disable_scroll_cb( GtkWidget *w ) {}

/* Disable scroll on a widget by adding a capture phase event handler and
 * connecting a no-op callback to the "scroll" event.
 */
GtkWidget *disable_scroll( GtkWidget *w )
{
        GtkEventController *ec;

        ec = gtk_event_controller_scroll_new(w,
                        GTK_EVENT_CONTROLLER_SCROLL_VERTICAL );

        gtk_event_controller_set_propagation_phase( ec, GTK_PHASE_CAPTURE );
        g_signal_connect( ec, "scroll", G_CALLBACK( disable_scroll_cb ), w );
        //gtk_widget_add_controller( w, ec );

        return w;
}

void enable_entry (GtkEntry *e) {
    gtk_widget_set_opacity (e, 1.0);
    gtk_widget_set_can_focus (e, TRUE);
    gtk_editable_set_editable (GTK_EDITABLE (e), TRUE);
}

void disable_entry (GtkEntry *e) {
    gtk_widget_set_opacity (e, 0.6);
    gtk_widget_set_can_focus (e, FALSE);
    gtk_editable_set_editable (GTK_EDITABLE (e), FALSE);
}