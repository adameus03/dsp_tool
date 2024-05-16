#include "controller_fir.h"
#include <gtk/gtk.h>

static struct ApplicationControls {
    GtkWidget* window;

    GtkWidget* comboBoxText_filterType;
    GtkWidget* comboBoxText_windowType;
    GtkWidget* entry_numCoeffs;

    GtkWidget* label_param1;
    GtkWidget* label_param2;
    GtkWidget* entry_param1val;
    GtkWidget* entry_param2val;

    GtkWidget* button_cancel;
    GtkWidget* button_executeFilter;

} widgets;

static struct ApplicationControlHelpers {
    //sem_t window_destroy_semph;
    controller_fir_callback_fn windowDestroyCb;
    gboolean isFirSubmitted;

    uint8_t filterType_idx; // for comboBoxText_filterType
} widget_helpers;

static struct ApplicationBuilders {
    GtkBuilder* viewFirBuilder;
} builders;



static void set_param_names(uint8_t filterTypeIdx) {
    switch (filterTypeIdx) {
        case 0U:
            gtk_label_set_text(GTK_LABEL(widgets.label_param1), "LPF cutoff frequency");
            gtk_widget_set_visible(widgets.label_param1, TRUE);
            gtk_widget_set_visible(widgets.label_param2, FALSE);
            gtk_widget_set_visible(widgets.entry_param1val, TRUE);
            gtk_widget_set_visible(widgets.entry_param2val, FALSE);
            break;
        case 1U:
            gtk_label_set_text(GTK_LABEL(widgets.label_param1), "HPF cutoff frequency");
            gtk_widget_set_visible(widgets.label_param1, TRUE);
            gtk_widget_set_visible(widgets.label_param2, FALSE);
            gtk_widget_set_visible(widgets.entry_param1val, TRUE);
            gtk_widget_set_visible(widgets.entry_param2val, FALSE);
            break;
        case 2U:
            gtk_label_set_text(GTK_LABEL(widgets.label_param1), "BPF left cutoff frequency");
            gtk_label_set_text(GTK_LABEL(widgets.label_param2), "BPF left cutoff frequency");
            gtk_widget_set_visible(widgets.label_param1, TRUE);
            gtk_widget_set_visible(widgets.label_param2, TRUE);
            gtk_widget_set_visible(widgets.entry_param1val, TRUE);
            gtk_widget_set_visible(widgets.entry_param2val, TRUE);
            break;
        default:
            g_error("Invalid filterTypeIdx");
            break;
    }
    
}

static uint8_t get_fir_filter_type_idx() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_filterType));
}

static uint8_t get_fir_windowing_window_type_idx() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_windowType));
}

static double get_num_coeffs() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entry_numCoeffs))); }

static double get_param1val() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entry_param1val))); }
static double get_param2val() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entry_param2val))); }


static fir_windowing_window_type_t get_windowing_window_type() {
    uint8_t windowing_window_type_idx = get_fir_windowing_window_type_idx();
    switch (windowing_window_type_idx)
    {
        case 0U:
            return FIR_WINDOWING_WINDOW_TYPE_RECTANGULAR;
        case 1U:
            return FIR_WINDOWING_WINDOW_TYPE_HAMMING;
        case 2U:
            return FIR_WINDOWING_WINDOW_TYPE_HANNING;
        case 3U:
            return FIR_WINDOWING_WINDOW_TYPE_BLACKMAN;
        default:
            g_error("Invalid windowing_window_type_idx detected");
            break;
    }
}

/**
 * [TODO] @verify for undefined behaviour
*/
static void on_window_destroyed() {
    g_message("Fir window destroyed.");
    if (widget_helpers.windowDestroyCb != NULL) {
        if (!widget_helpers.isFirSubmitted) { 
            fir_common_config_t rejectionFirConfig = {
                .filterType = -1
            };
            widget_helpers.windowDestroyCb(rejectionFirConfig);
            return; 
        }

        fir_common_config_t firConfig = {};
        switch (widget_helpers.filterType_idx) {
            case 0U:
                firConfig.filterType = FIR_FILTER_TYPE_LOWPASS;
                firConfig.oneSidedConfig = (fir_one_sided_real_filter_config_t){
                    .cutoff_frequency = get_param1val(),
                    .windowing.num_fir_coeffs = get_num_coeffs(),
                    .windowing.window_type = get_windowing_window_type()
                };
                break;
            case 1U:
                firConfig.filterType = FIR_FILTER_TYPE_HIGHPASS;
                firConfig.oneSidedConfig = (fir_one_sided_real_filter_config_t){
                    .cutoff_frequency = get_param1val(),
                    .windowing.num_fir_coeffs = get_num_coeffs(),
                    .windowing.window_type = get_windowing_window_type()
                };
                break;
            case 2U:
                firConfig.filterType = FIR_FILTER_TYPE_BANDPASS;
                firConfig.doubleSidedConfig = (fir_double_sided_real_filter_config_t){
                    .left_cutoff_frequency = get_param1val(),
                    .right_cutoff_frequency = get_param2val(),
                    .windowing.num_fir_coeffs = get_num_coeffs(),
                    .windowing.window_type = get_windowing_window_type()
                };
                break;
            default:
                g_error("Invalid widget_helpers.filterType_idx detected");
                break;
        }

        widget_helpers.windowDestroyCb(firConfig);
    }
}

int controller_fir_run(controller_fir_callback_fn windowDestroyCallback) {
    builders.viewFirBuilder = gtk_builder_new_from_file("view_fir.xml");
    widgets.window = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "window"));

    widgets.comboBoxText_filterType = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "comboBoxText_filterType"));
    widgets.comboBoxText_windowType = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "comboBoxText_windowType"));
    widgets.entry_numCoeffs = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "entry_numCoeffs"));

    widgets.label_param1 = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "label_param1"));
    widgets.label_param2 = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "label_param2"));
    widgets.entry_param1val = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "entry_param1val"));
    widgets.entry_param2val = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "entry_param2val"));

    widgets.button_cancel = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "button_cancel"));
    widgets.button_executeFilter = GTK_WIDGET(gtk_builder_get_object(builders.viewFirBuilder, "button_executeFilter"));
    
    widget_helpers.isFirSubmitted = FALSE;
    widget_helpers.windowDestroyCb = windowDestroyCallback;
    widget_helpers.filterType_idx = 0U;


    set_param_names(get_fir_filter_type_idx());

    gtk_builder_connect_signals(builders.viewFirBuilder, NULL);

    gtk_widget_show(widgets.window);

    g_signal_connect (widgets.window, "destroy", G_CALLBACK(on_window_destroyed), NULL);

    return EXIT_SUCCESS;
}

void on_comboBoxText_filterType_changed(GtkComboBox* c, gpointer user_data) {
    widget_helpers.filterType_idx = gtk_combo_box_get_active(c);
    set_param_names(widget_helpers.filterType_idx);
}

void on_button_cancel_clicked(GtkButton* b) {
    widget_helpers.isFirSubmitted = FALSE;
    gtk_window_close(GTK_WINDOW(widgets.window));
}

void on_button_executeFilter_clicked(GtkButton* b) {
    widget_helpers.isFirSubmitted = TRUE;
    gtk_window_close(GTK_WINDOW(widgets.window));
}