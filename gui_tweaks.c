#include "gui_tweaks.h"

/* No-op to prevent `w` from propagating "scroll" events it receives.
 */
void disable_scroll_cb( GtkWidget *w ) {}

/* Disable scroll on a widget by adding a capture phase event handler and
 * connecting a no-op callback to the "scroll" event.
    @source : https://stackoverflow.com/questions/72832314/how-to-disable-scrollwheel-on-gtk-scale-widget%20
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
    gtk_widget_set_opacity (GTK_WIDGET(e), 1.0);
    gtk_widget_set_can_focus (GTK_WIDGET(e), TRUE);
    gtk_editable_set_editable (GTK_EDITABLE (e), TRUE);
}

void disable_entry (GtkEntry *e) {
    gtk_widget_set_opacity (GTK_WIDGET(e), 0.6);
    gtk_widget_set_can_focus (GTK_WIDGET(e), FALSE);
    gtk_editable_set_editable (GTK_EDITABLE (e), FALSE);
}

void enable_combo_box( GtkComboBox* c ) {
    gtk_widget_set_opacity (GTK_WIDGET(c), 1.0);
    gtk_widget_set_can_focus (GTK_WIDGET(c), TRUE);
}

void disable_combo_box( GtkComboBox* c ) {
    gtk_widget_set_opacity (GTK_WIDGET(c), 0.6);
    gtk_widget_set_can_focus (GTK_WIDGET(c), FALSE);
}

void enable_button( GtkButton* b ) {
    gtk_widget_set_opacity (GTK_WIDGET(b), 1.0);
    gtk_widget_set_can_focus (GTK_WIDGET(b), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(b), TRUE);
}

void disable_button( GtkButton* b ) {
    gtk_widget_set_opacity (GTK_WIDGET(b), 0.6);
    gtk_widget_set_can_focus (GTK_WIDGET(b), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(b), FALSE);
}

void pseudo_disable_window( GtkWindow* w ) {
    gtk_widget_set_opacity (GTK_WIDGET(w), 0.6);
    gtk_widget_set_can_focus (GTK_WIDGET(w), FALSE);
}

void pseudo_enable_window( GtkWindow* w ) {
    gtk_widget_set_opacity (GTK_WIDGET(w), 1.0);
    gtk_widget_set_can_focus (GTK_WIDGET(w), TRUE);
}