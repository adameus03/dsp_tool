#include "controller_timeshift.h"
#include <gtk/gtk.h>
#include <semaphore.h>

static struct ApplicationControls {
    GtkWidget* window;

    GtkWidget* entryTimeshift;
    GtkWidget* buttonCancel;
    GtkWidget* buttonExecuteShift;
} widgets;

static struct ApplicationControlHelpers {
    sem_t window_destroy_semph;
} widget_helpers;

static struct ApplicationBuilders {
    GtkBuilder* viewTimeshiftBuilder;
} builders;

int controller_timeshift_run() {
    builders.viewTimeshiftBuilder = gtk_builder_new_from_file("view_timeshift.xml");
    widgets.window = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "window"));
    
    widgets.entryTimeshift = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "entryTimeshift"));
    widgets.buttonCancel = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "buttonCancel"));
    widgets.buttonExecuteShift = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "buttonExecuteShift"));
    
    gtk_builder_connect_signals(builders.viewTimeshiftBuilder, NULL);

    gtk_widget_show(widgets.window);

    g_signal_connect (widgets.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    return EXIT_SUCCESS;
}

void on_buttonCancel_clicked(GtkButton* b) {

}

void on_buttonExecuteShift_clicked(GtkButton* b) {
    
}