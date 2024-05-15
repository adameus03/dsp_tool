#include "controller_timeshift.h"
#include <gtk/gtk.h>
#include <semaphore.h>
#include <errno.h>

static struct ApplicationControls {
    GtkWidget* window;

    GtkWidget* entryTimeshift;
    GtkWidget* buttonCancel;
    GtkWidget* buttonExecuteShift;
} widgets;

static struct ApplicationControlHelpers {
    //sem_t window_destroy_semph;
    timeshift_callback_fn windowDestroyCb;
    gboolean isTimeshiftSubmitted;
} widget_helpers;

static struct ApplicationBuilders {
    GtkBuilder* viewTimeshiftBuilder;
} builders;

static double get_timeshiftval() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entryTimeshift))); }

/**
 * [TODO] @verify for undefined behaviour
*/
static void on_window_destroyed() {
    g_message("Timeshift window destroyed.");
    /*if (-1 == sem_post(&widget_helpers.window_destroy_semph)) {
        g_error("sem_post failed [errno=%d]", errno);
    }*/
    if (widget_helpers.windowDestroyCb != NULL) {
        double timeshiftVal = widget_helpers.isTimeshiftSubmitted ? get_timeshiftval() : 0;
        widget_helpers.windowDestroyCb(timeshiftVal);
    }
}

int controller_timeshift_run(timeshift_callback_fn windowDestroyCallback) {
    builders.viewTimeshiftBuilder = gtk_builder_new_from_file("view_timeshift.xml");
    widgets.window = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "window"));
    
    widgets.entryTimeshift = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "entryTimeshift"));
    widgets.buttonCancel = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "buttonCancel"));
    widgets.buttonExecuteShift = GTK_WIDGET(gtk_builder_get_object(builders.viewTimeshiftBuilder, "buttonExecuteShift"));

    widget_helpers.isTimeshiftSubmitted = FALSE;
    widget_helpers.windowDestroyCb = windowDestroyCallback;
    
    gtk_builder_connect_signals(builders.viewTimeshiftBuilder, NULL);

    gtk_widget_show(widgets.window);


    g_signal_connect (widgets.window, "destroy", G_CALLBACK(on_window_destroyed), NULL);
    /*if (-1 == sem_init(&widget_helpers.window_destroy_semph, 0, 1)) {
        g_error("sem_init failed [errno=%d]", errno);
    }
    g_signal_connect (widgets.window, "destroy", G_CALLBACK(on_window_destroyed), NULL);
    if (-1 == sem_wait(&widget_helpers.window_destroy_semph)) {
        g_error("sem_wait failed [errno=%d]", errno);
    }*/
    /*if (-1 == sem_destroy(&widget_helpers.window_destroy_semph)) {
        g_error("sem_destroy failed [errno=%d]", errno);
    }*/
    //return &widget_helpers.window_destroy_semph;
    return EXIT_SUCCESS;
}

/*void controller_timeshift_cleanup() {
    if (-1 == sem_destroy(&widget_helpers.window_destroy_semph)) {
        g_error("sem_destroy failed [errno=%d]", errno);
    }
}*/

void on_buttonCancel_clicked(GtkButton* b) {
    widget_helpers.isTimeshiftSubmitted = FALSE;
    gtk_window_close(GTK_WINDOW(widgets.window));
}

void on_buttonExecuteShift_clicked(GtkButton* b) {
    widget_helpers.isTimeshiftSubmitted = TRUE;
    gtk_window_close(GTK_WINDOW(widgets.window));
}
